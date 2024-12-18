#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define TAB "    "
#define TABS 4

int num_len(size_t num) {
    int len = 0;
    while (num != 0) {
        num /= 10;
        len++;
    }
    return len ? len : 1;
}

struct entry {
    char fname[256];
    char uname[256];
    char gname[256];
    struct stat st;
    struct tm mtime;
};

struct entry *read_entries(int n_entries, char **names) {
    struct stat st;
    struct passwd *pwd;
    struct group *grp;

    struct entry *entries = malloc(n_entries * sizeof(struct entry));

    for (size_t i = 0; i < n_entries; i++) {
        if (stat(names[i], &st) == -1) {
            printf("can't stat %s: %s\n", names[i], strerror(errno));
            exit(1);
        }

        pwd = getpwuid(st.st_uid);
        if (pwd == NULL) {
            printf("can't get passwd for uid %d: %s\n", st.st_uid,
                   strerror(errno));
            exit(1);
        }

        grp = getgrgid(st.st_gid);
        if (grp == NULL) {
            printf("can't get group for gid %d: %s\n", st.st_gid,
                   strerror(errno));
            exit(1);
        }

        strcpy(entries[i].fname, names[i]);
        strcpy(entries[i].uname, pwd->pw_name);
        strcpy(entries[i].gname, grp->gr_name);
        entries[i].st = st;
        localtime_r(&st.st_mtime, &entries[i].mtime);
        entries[i].mtime.tm_year += 1900;
    }

    return entries;
}

const char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

struct col_info {
    int max_size_len;
    int max_fname_len;
    int max_uname_len;
    int max_gname_len;
    char has_files;
    char show_md;
    char show_year;
    int max_day_len;
    int max_year_len;
    int max_links_len;
};

struct col_info get_col_info(struct tm ctm, struct entry *entries, size_t n) {
    struct col_info info = {0};
    info.max_uname_len = 3;
    info.max_gname_len = 3;
    info.max_size_len = 4;
    info.max_year_len = 4;
    info.max_links_len = 4;

    for (size_t i = 0; i < n; i++) {
        struct entry *cur = &entries[i];

        if (S_ISREG(cur->st.st_mode)) {
            info.has_files = 1;
            if (num_len(cur->st.st_size) > info.max_size_len) {
                info.max_size_len = num_len(cur->st.st_size);
            }
        }

        size_t uname_len = strlen(cur->uname);
        if (info.max_uname_len < uname_len) {
            info.max_uname_len = uname_len;
        }

        size_t gname_len = strlen(cur->gname);
        if (info.max_gname_len < gname_len) {
            info.max_gname_len = gname_len;
        }

        size_t fname_len = strlen(cur->fname);
        if (info.max_fname_len < fname_len) {
            info.max_fname_len = fname_len;
        }

        if (cur->mtime.tm_year != ctm.tm_year) {
            info.show_year = 1;
        }

        if (cur->mtime.tm_year != ctm.tm_year ||
            cur->mtime.tm_yday != ctm.tm_yday) {
            info.show_md = 1;
        }

        int day_len = num_len(cur->mtime.tm_mday);
        if (info.max_day_len < day_len) {
            info.max_day_len = day_len;
        }

        int year_len = num_len(cur->mtime.tm_year);
        if (info.max_year_len < year_len) {
            info.max_year_len = year_len;
        }

        int links_len = num_len(cur->st.st_nlink);
        if (info.max_links_len < links_len) {
            info.max_links_len = links_len;
        }
    }

    return info;
}

int main(int argc, char *argv[]) {
    time_t ctime = time(NULL);
    struct tm ctm;
    localtime_r(&ctime, &ctm);
    ctm.tm_year += 1900;

    struct entry *entries;
    size_t n_entries;
    if (argc == 1) {
        char *args = ".";
        entries = read_entries(1, &args);
        n_entries = 1;
    } else {
        n_entries = argc - 1;
        entries = read_entries(n_entries, argv + 1);
    }

    struct col_info info = get_col_info(ctm, entries, n_entries);

    // header
    printf("     flags" TAB "%*s" TAB "%*s", info.max_uname_len, "uid",
           info.max_gname_len, "gid");

    if (info.has_files) {
        printf(TAB "%*s", info.max_size_len, "size");
    }

    printf(TAB "%*s", info.max_links_len, "link");

    if (info.show_md) {
        printf(TAB "%*s",
               4 + info.max_day_len + info.show_year * (info.max_year_len + 1),
               "date");
    }

    printf(TAB " time" TAB "name\n");

    for (size_t i = 0; i < n_entries; i++) {
        struct entry *cur = &entries[i];

        char perms[11];

        if (S_ISDIR(cur->st.st_mode)) {
            perms[0] = 'd';
        } else if (S_ISREG(cur->st.st_mode)) {
            perms[0] = '-';
        } else {
            perms[0] = '?';
        }
        perms[1] = cur->st.st_mode & S_IRUSR ? 'r' : '-';
        perms[2] = cur->st.st_mode & S_IWUSR ? 'w' : '-';
        perms[3] = cur->st.st_mode & S_IXUSR ? 'x' : '-';
        perms[4] = cur->st.st_mode & S_IRGRP ? 'r' : '-';
        perms[5] = cur->st.st_mode & S_IWGRP ? 'w' : '-';
        perms[6] = cur->st.st_mode & S_IXGRP ? 'x' : '-';
        perms[7] = cur->st.st_mode & S_IROTH ? 'r' : '-';
        perms[8] = cur->st.st_mode & S_IWOTH ? 'w' : '-';
        perms[9] = cur->st.st_mode & S_IXOTH ? 'x' : '-';
        perms[10] = 0;

        printf("%s" TAB "%*s" TAB "%*s", perms, info.max_uname_len, cur->uname,
               info.max_gname_len, cur->gname);

        if (S_ISREG(cur->st.st_mode)) {
            printf(TAB "%*zu", info.max_size_len, cur->st.st_size);
        } else if (info.has_files) {
            printf(TAB "%*s", info.max_size_len, "");
        }

        printf(TAB "%*zu", info.max_links_len, cur->st.st_nlink);

        if (info.show_md) {
            printf(TAB "%s %*d", months[cur->mtime.tm_mon], info.max_day_len,
                   cur->mtime.tm_mday);
        }

        if (info.show_year) {
            printf(" %*d", info.max_year_len, cur->mtime.tm_year);
        }

        printf(TAB "%02d:%02d" TAB "%s\n", cur->mtime.tm_hour,
               cur->mtime.tm_min, cur->fname);
    }
}