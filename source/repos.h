#pragma once
#include <stdbool.h>
#include <stddef.h>

// A GitHub repository whose latest release provides a .nds file
struct Repo {
    const char* name; // Display name
    const char* slug; // "owner/repository"
};

typedef enum {
    REPO_RELEASE_SUCCESS,
    REPO_RELEASE_ERR_WIFI_NOT_CONNECTED,
    REPO_RELEASE_ERR_REQUEST_FAILED,
    REPO_RELEASE_ERR_NO_NDS_FILE
} RepoReleaseStatus;

struct RepoRelease {
    char tag[64];
    char assetName[128];
    char assetUrl[512];
};

const struct Repo* getRepos(size_t* reposCount);

// Queries GitHub for the latest release of the repo and finds its first .nds asset
RepoReleaseStatus fetchLatestRepoRelease(const struct Repo* repo, struct RepoRelease* release);

// Release tag that was last installed for the repo (empty if never installed)
bool getInstalledRepoTag(const struct Repo* repo, char* tag, size_t tagSize);
bool setInstalledRepoTag(const struct Repo* repo, const char* tag);
