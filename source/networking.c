#include "networking.h"

#include "config.h"
#include <dswifi9.h>
#include <nds.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE (32 * 1024) // 32 KiB - DS has limited RAM
#define USER_AGENT APP_NAME "/" APP_VERSION " (DS)"

static bool wifiInitialized = false;

static bool waitForWifiConnection(void)
{
    if (!wifiInitialized)
        return false;

    int status = Wifi_AssocStatus();
    if (status == ASSOCSTATUS_ASSOCIATED)
        return true;

    if (status == ASSOCSTATUS_DISCONNECTED || status == ASSOCSTATUS_CANNOTCONNECT)
        Wifi_AutoConnect();

    for (u32 frame = 0; frame < 600; frame++) {
        status = Wifi_AssocStatus();
        if (status == ASSOCSTATUS_ASSOCIATED)
            return true;
        if (status == ASSOCSTATUS_CANNOTCONNECT)
            return false;

        swiWaitForVBlank();
    }

    return false;
}

NetworkingInitStatus initNetworking(void)
{
    if (!Wifi_InitDefault(INIT_ONLY | WIFI_ATTEMPT_DSI_MODE))
        return NETWORKING_INIT_ERR_WIFI_CONNECT;

    wifiInitialized = true;
    Wifi_AutoConnect();
    curl_global_init(CURL_GLOBAL_DEFAULT);

    return NETWORKING_INIT_SUCCESS;
}

static bool stopDownloadSignal = false;

struct WriteData {
    FILE* fp;
    char* buffer;
    size_t bufferPos;
};

static size_t writeDataCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    struct WriteData* writeData = (struct WriteData*)userdata;
    if (stopDownloadSignal)
        return -1;

    size_t total_size = size * nmemb;
    if (writeData->bufferPos + total_size > BUFFER_SIZE) {
        fwrite(writeData->buffer, 1, writeData->bufferPos, writeData->fp);
        writeData->bufferPos = 0;
    }

    if (total_size > BUFFER_SIZE) {
        fwrite(ptr, 1, total_size, writeData->fp);
    } else {
        memcpy(writeData->buffer + writeData->bufferPos, ptr, total_size);
        writeData->bufferPos += total_size;
    }

    return total_size;
}

static DownloadStatus downloadFileInternal(const char* path, const char* url, size_t (*downloadProgressCallback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t), DownloadFrameCallback frameCallback)
{
    stopDownloadSignal = false;

    if (!waitForWifiConnection())
        return DOWNLOAD_ERR_WIFI_NOT_CONNECTED;

    CURL* curl = curl_easy_init();
    if (!curl)
        return DOWNLOAD_ERR_INIT_FAILED;

    FILE* fp = fopen(path, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return DOWNLOAD_ERR_FILE_OPEN_FAILED;
    }

    struct WriteData writeData;
    writeData.fp = fp;
    writeData.buffer = (char*)malloc(BUFFER_SIZE);
    writeData.bufferPos = 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7L);

    if (downloadProgressCallback) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, downloadProgressCallback);
    }

    CURLcode res = CURLE_OK;
    if (frameCallback) {
        CURLM* multi = curl_multi_init();
        if (!multi) {
            free(writeData.buffer);
            fclose(fp);
            curl_easy_cleanup(curl);
            unlink(path);
            return DOWNLOAD_ERR_INIT_FAILED;
        }

        CURLMcode multiStatus = curl_multi_add_handle(multi, curl);
        if (multiStatus != CURLM_OK) {
            curl_multi_cleanup(multi);
            free(writeData.buffer);
            fclose(fp);
            curl_easy_cleanup(curl);
            unlink(path);
            return DOWNLOAD_ERR_INIT_FAILED;
        }

        int runningHandles = 0;
        do {
            do {
                multiStatus = curl_multi_perform(multi, &runningHandles);
            } while (multiStatus == CURLM_CALL_MULTI_PERFORM);

            if (multiStatus != CURLM_OK) {
                res = CURLE_FAILED_INIT;
                break;
            }

            if (runningHandles > 0 && !stopDownloadSignal)
                frameCallback();
        } while (runningHandles > 0 && !stopDownloadSignal);

        int messagesLeft = 0;
        CURLMsg* message;
        while ((message = curl_multi_info_read(multi, &messagesLeft)) != NULL) {
            if (message->msg == CURLMSG_DONE)
                res = message->data.result;
        }

        curl_multi_remove_handle(multi, curl);
        curl_multi_cleanup(multi);
    } else {
        res = curl_easy_perform(curl);
    }

    if (writeData.bufferPos > 0)
        fwrite(writeData.buffer, 1, writeData.bufferPos, writeData.fp);

    free(writeData.buffer);
    fclose(fp);

    if (stopDownloadSignal) {
        curl_easy_cleanup(curl);
        unlink(path);
        return DOWNLOAD_STOPPED;
    }

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        unlink(path);
        return DOWNLOAD_ERR_PERFORM_FAILED;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode != 200) {
        unlink(path);
        return DOWNLOAD_ERR_NOT_OK;
    }

    return DOWNLOAD_SUCCESS;
}

DownloadStatus downloadFile(const char* path, const char* url, size_t (*downloadProgressCallback)(void*, curl_off_t, curl_off_t, curl_off_t, curl_off_t))
{
    return downloadFileInternal(path, url, downloadProgressCallback, NULL);
}

DownloadStatus downloadFilePumped(const char* path, const char* url, DownloadFrameCallback frameCallback)
{
    return downloadFileInternal(path, url, NULL, frameCallback);
}

void stopDownload(void)
{
    stopDownloadSignal = true;
}

struct MemoryWriteData {
    char* result;
    size_t bufferSize;
    size_t currentPos;
};

static size_t memoryWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    struct MemoryWriteData* writeData = (struct MemoryWriteData*)userdata;
    size_t total_size = size * nmemb;

    if (writeData->currentPos + total_size >= writeData->bufferSize) {
        total_size = writeData->bufferSize - writeData->currentPos - 1;
    }

    memcpy(writeData->result + writeData->currentPos, ptr, total_size);
    writeData->currentPos += total_size;
    writeData->result[writeData->currentPos] = '\0';

    return total_size;
}

DownloadStatus downloadToString(char* result, size_t bufferSize, const char* url)
{
    if (!waitForWifiConnection())
        return DOWNLOAD_ERR_WIFI_NOT_CONNECTED;

    CURL* curl = curl_easy_init();
    if (!curl)
        return DOWNLOAD_ERR_INIT_FAILED;

    struct MemoryWriteData writeData;
    writeData.result = result;
    writeData.bufferSize = bufferSize;
    writeData.currentPos = 0;
    result[0] = '\0';

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memoryWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeData);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7L);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        return DOWNLOAD_ERR_PERFORM_FAILED;
    }

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_easy_cleanup(curl);

    if (httpCode != 200)
        return DOWNLOAD_ERR_NOT_OK;

    return DOWNLOAD_SUCCESS;
}
