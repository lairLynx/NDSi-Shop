#include "repos.h"

#include "config.h"
#include "networking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REPOS_FILENAME "repos.cfg"
#define REPOS_PATH APPDATA_DIR "/" REPOS_FILENAME
#define RELEASE_RESPONSE_SIZE (48 * 1024)

// Repositories shown in repo mode
static const struct Repo repos[] = {
    { "Lynxibear-DS", "lairLynx/Lynxibear-DS" },
};

const struct Repo* getRepos(size_t* reposCount)
{
    *reposCount = sizeof(repos) / sizeof(repos[0]);
    return repos;
}

// Finds the string value of a JSON key at or after `from`, e.g. "tag_name": "1.0"
// Returns a pointer just past the value, or NULL if the key was not found
static const char* findJsonString(const char* from, const char* key, char* value, size_t valueSize)
{
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);

    const char* ptr = strstr(from, pattern);
    if (!ptr)
        return NULL;

    ptr += strlen(pattern);
    while (*ptr == ' ' || *ptr == ':')
        ptr++;

    if (*ptr != '"')
        return NULL;
    ptr++;

    const char* end = strchr(ptr, '"');
    if (!end || (size_t)(end - ptr) >= valueSize)
        return NULL;

    memcpy(value, ptr, end - ptr);
    value[end - ptr] = '\0';

    return end + 1;
}

static bool endsWithNds(const char* str)
{
    size_t length = strlen(str);
    if (length < 4)
        return false;

    const char* ext = str + length - 4;
    return ext[0] == '.' && (ext[1] == 'n' || ext[1] == 'N') && (ext[2] == 'd' || ext[2] == 'D') && (ext[3] == 's' || ext[3] == 'S');
}

RepoReleaseStatus fetchLatestRepoRelease(const struct Repo* repo, struct RepoRelease* release)
{
    char url[160];
    snprintf(url, sizeof(url), "https://api.github.com/repos/%s/releases/latest", repo->slug);

    char* response = malloc(RELEASE_RESPONSE_SIZE);
    if (!response)
        return REPO_RELEASE_ERR_REQUEST_FAILED;

    DownloadStatus status = downloadToString(response, RELEASE_RESPONSE_SIZE, url);
    if (status != DOWNLOAD_SUCCESS) {
        free(response);
        return status == DOWNLOAD_ERR_WIFI_NOT_CONNECTED ? REPO_RELEASE_ERR_WIFI_NOT_CONNECTED : REPO_RELEASE_ERR_REQUEST_FAILED;
    }

    RepoReleaseStatus result = REPO_RELEASE_ERR_NO_NDS_FILE;

    release->tag[0] = '\0';
    findJsonString(response, "tag_name", release->tag, sizeof(release->tag));

    // Take the first asset whose download URL points to a .nds file
    const char* ptr = response;
    while ((ptr = findJsonString(ptr, "browser_download_url", release->assetUrl, sizeof(release->assetUrl))) != NULL) {
        if (!endsWithNds(release->assetUrl))
            continue;

        const char* fileName = strrchr(release->assetUrl, '/');
        fileName = fileName ? fileName + 1 : release->assetUrl;
        if (strlen(fileName) >= sizeof(release->assetName))
            continue;

        strcpy(release->assetName, fileName);
        result = REPO_RELEASE_SUCCESS;
        break;
    }

    free(response);
    return result;
}

bool getInstalledRepoTag(const struct Repo* repo, char* tag, size_t tagSize)
{
    tag[0] = '\0';

    FILE* fp = fopen(REPOS_PATH, "r");
    if (!fp)
        return false;

    size_t nameLength = strlen(repo->name);
    char line[256];
    bool found = false;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, repo->name, nameLength) == 0 && line[nameLength] == '=') {
            snprintf(tag, tagSize, "%s", line + nameLength + 1);
            found = true;
            break;
        }
    }

    fclose(fp);
    return found;
}

bool setInstalledRepoTag(const struct Repo* repo, const char* tag)
{
    // Rewrite the file, keeping the entries of the other repos
    size_t reposCount;
    const struct Repo* all = getRepos(&reposCount);

    char tags[8][64];
    if (reposCount > 8)
        reposCount = 8;
    for (size_t i = 0; i < reposCount; i++) {
        if (&all[i] == repo)
            snprintf(tags[i], sizeof(tags[i]), "%s", tag);
        else
            getInstalledRepoTag(&all[i], tags[i], sizeof(tags[i]));
    }

    FILE* fp = fopen(REPOS_PATH, "w");
    if (!fp)
        return false;

    for (size_t i = 0; i < reposCount; i++) {
        if (tags[i][0] != '\0')
            fprintf(fp, "%s=%s\n", all[i].name, tags[i]);
    }

    fclose(fp);
    return true;
}
