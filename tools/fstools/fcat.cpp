/*
** fcat
** The Sleuth Kit
**
** Brian Carrier [carrier <at> sleuthkit [dot] org]
** Copyright (c) 2012 Brian Carrier, Basis Technology.  All Rights reserved
**
** This software is distributed under the Common Public License 1.0
**/

#include "tsk/tsk_tools_i.h"
#include "tsk/base/tsk_os_cpp.h"

#include <locale.h>

#include <memory>

/* usage - explain and terminate */

static TSK_TCHAR *progname;

static void
usage()
{
    TFPRINTF(stderr,
        _TSK_T
        ("usage: %" PRIttocTSK " [-hRsvV] [-f fstype] [-i imgtype] [-b dev_sector_size] [-o imgoffset] [-P pooltype] [-B pool_volume_block] file_path image [images]\n"),
        progname);
    tsk_fprintf(stderr, "\t-h: Do not display holes in sparse files\n");
    tsk_fprintf(stderr,
        "\t-R: Suppress recovery errors\n");
    tsk_fprintf(stderr, "\t-s: Display slack space at end of file\n");
    tsk_fprintf(stderr,
        "\t-i imgtype: The format of the image file (use '-i list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-b dev_sector_size: The size (in bytes) of the device sectors\n");
    tsk_fprintf(stderr,
        "\t-f fstype: File system type (use '-f list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-o imgoffset: The offset of the file system in the image (in sectors)\n");
    tsk_fprintf(stderr,
        "\t-P pooltype: Pool container type (use '-p list' for supported types)\n");
    tsk_fprintf(stderr,
        "\t-B pool_volume_block: Starting block (for pool volumes only)\n");
    tsk_fprintf(stderr, "\t-v: verbose to stderr\n");
    tsk_fprintf(stderr, "\t-V: Print version\n");

    exit(1);
}

int
main(int argc, char **argv1)
{
    TSK_IMG_TYPE_ENUM imgtype = TSK_IMG_TYPE_DETECT;

    TSK_OFF_T imgaddr = 0;
    TSK_FS_TYPE_ENUM fstype = TSK_FS_TYPE_DETECT;

    TSK_POOL_TYPE_ENUM pooltype = TSK_POOL_TYPE_DETECT;
    TSK_OFF_T pvol_block = 0;
    const char * password = ""; // Not currently used

    TSK_INUM_T inum;
    int fw_flags = 0;
    int ch;
    int retval;
    int suppress_recover_error = 0;
    TSK_TCHAR **argv;
    TSK_TCHAR *cp;
    unsigned int ssize = 0;
    TSK_TSTRING path;

#ifdef TSK_WIN32
    // On Windows, get the wide arguments (mingw doesn't support wmain)
    argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) {
        fprintf(stderr, "Error getting wide arguments\n");
        exit(1);
    }
#else
    argv = (TSK_TCHAR **) argv1;
#endif

    progname = argv[0];
    setlocale(LC_ALL, "");

    while ((ch = GETOPT(argc, argv, _TSK_T("b:B:f:hi:o:P:rRsvV"))) > 0) {
        switch (ch) {
        case _TSK_T('?'):
        default:
            TFPRINTF(stderr, _TSK_T("Invalid argument: %" PRIttocTSK "\n"),
                argv[OPTIND]);
            usage();
            break;
        case _TSK_T('b'):
            ssize = (unsigned int) TSTRTOUL(OPTARG, &cp, 0);
            if (*cp || *cp == *OPTARG || ssize < 1) {
                TFPRINTF(stderr,
                    _TSK_T
                    ("invalid argument: sector size must be positive: %" PRIttocTSK "\n"),
                    OPTARG);
                usage();
            }
            break;
        case _TSK_T('f'):
            if (TSTRCMP(OPTARG, _TSK_T("list")) == 0) {
                tsk_fs_type_print(stderr);
                exit(1);
            }
            fstype = tsk_fs_type_toid(OPTARG);
            if (fstype == TSK_FS_TYPE_UNSUPP) {
                TFPRINTF(stderr,
                    _TSK_T("Unsupported file system type: %" PRIttocTSK "\n"), OPTARG);
                usage();
            }
            break;
        case _TSK_T('h'):
            fw_flags |= TSK_FS_FILE_WALK_FLAG_NOSPARSE;
            break;
        case _TSK_T('i'):
            if (TSTRCMP(OPTARG, _TSK_T("list")) == 0) {
                tsk_img_type_print(stderr);
                exit(1);
            }
            imgtype = tsk_img_type_toid(OPTARG);
            if (imgtype == TSK_IMG_TYPE_UNSUPP) {
                TFPRINTF(stderr, _TSK_T("Unsupported image type: %" PRIttocTSK "\n"),
                    OPTARG);
                usage();
            }
            break;
        case _TSK_T('o'):
            if ((imgaddr = tsk_parse_offset(OPTARG)) == -1) {
                tsk_error_print(stderr);
                exit(1);
            }
            break;
        case _TSK_T('P'):
            if (TSTRCMP(OPTARG, _TSK_T("list")) == 0) {
                tsk_pool_type_print(stderr);
                exit(1);
            }
            pooltype = tsk_pool_type_toid(OPTARG);
            if (pooltype == TSK_POOL_TYPE_UNSUPP) {
                TFPRINTF(stderr,
                    _TSK_T("Unsupported pool container type: %s\n"), OPTARG);
                usage();
            }
            break;
        case _TSK_T('B'):
            if ((pvol_block = tsk_parse_offset(OPTARG)) == -1) {
                tsk_error_print(stderr);
                exit(1);
            }
            break;
        case _TSK_T('R'):
            suppress_recover_error = 1;
            break;
        case _TSK_T('s'):
            fw_flags |= TSK_FS_FILE_WALK_FLAG_SLACK;
            break;
        case _TSK_T('v'):
            tsk_verbose++;
            break;
        case _TSK_T('V'):
            tsk_version_print(stdout);
            exit(0);
        }
    }

    /* We need at least two more arguments */
    if (OPTIND + 1 >= argc) {
        tsk_fprintf(stderr, "Missing image name and/or path\n");
        usage();
    }

    // copy in path
    path = argv[OPTIND];

    std::unique_ptr<TSK_IMG_INFO, decltype(&tsk_img_close)> img{
        tsk_img_open(argc - OPTIND - 1, &argv[OPTIND+1], imgtype, ssize),
        tsk_img_close
    };

    if (!img) {
        tsk_error_print(stderr);
        exit(1);
    }

    if (imgaddr * img->sector_size >= img->size) {
        tsk_fprintf(stderr,
            "Sector offset supplied is larger than disk image (maximum: %"
            PRIu64 ")\n", img->size / img->sector_size);
        exit(1);
    }

    std::unique_ptr<const TSK_POOL_INFO, decltype(&tsk_pool_close)> pool{
        nullptr,
        tsk_pool_close
    };

    std::unique_ptr<TSK_IMG_INFO, decltype(&tsk_img_close)> pool_img{
        nullptr,
        tsk_img_close
    };

    std::unique_ptr<TSK_FS_INFO, decltype(&tsk_fs_close)> fs{
        nullptr,
        tsk_fs_close
    };

    if (pvol_block == 0) {
        fs.reset(tsk_fs_open_img_decrypt(img.get(), imgaddr * img->sector_size, fstype, password));
        if (!fs) {
            tsk_error_print(stderr);
            if (tsk_error_get_errno() == TSK_ERR_FS_UNSUPTYPE)
                tsk_fs_type_print(stderr);
            exit(1);
        }
    }
    else {
        pool.reset(tsk_pool_open_img_sing(img.get(), imgaddr * img->sector_size, pooltype));
        if (!pool) {
            tsk_error_print(stderr);
            if (tsk_error_get_errno() == TSK_ERR_FS_UNSUPTYPE)
                tsk_pool_type_print(stderr);
            exit(1);
        }

        TSK_OFF_T offset = imgaddr * img->sector_size;
#if HAVE_LIBVSLVM
        if (pool->ctype == TSK_POOL_TYPE_LVM){
            offset = 0;
        }
#endif /* HAVE_LIBVSLVM */
        pool_img.reset(pool->get_img_info(pool.get(), (TSK_DADDR_T)pvol_block));
        // FIXME: Will crash if !pool_img
        fs.reset(tsk_fs_open_img_decrypt(pool_img.get(), offset, fstype, password));
        if (!fs) {
            tsk_error_print(stderr);
            if (tsk_error_get_errno() == TSK_ERR_FS_UNSUPTYPE)
                tsk_fs_type_print(stderr);
            exit(1);
        }
    }

    if (-1 == (retval = tsk_fs_ifind_path(fs.get(), &path[0], &inum))) {
        tsk_error_print(stderr);
        exit(1);
    }
    else if (retval == 1) {
        tsk_fprintf(stderr, "File not found\n");
        exit(1);
    }

    // @@@ Cannot currently get ADS with this approach
    retval =
        tsk_fs_icat(fs.get(), inum, (TSK_FS_ATTR_TYPE_ENUM)0, 0, 0, 0,
        (TSK_FS_FILE_WALK_FLAG_ENUM) fw_flags);
    if (retval) {
        if (suppress_recover_error == 1
            && tsk_error_get_errno() == TSK_ERR_FS_RECOVER) {
            tsk_error_reset();
        }
        else {
            tsk_error_print(stderr);
            exit(1);
        }
    }

    exit(0);
}
