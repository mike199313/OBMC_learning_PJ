PACKAGECONFIG[disable-everything] = "--disable-everything,"
PACKAGECONFIG[muxer-avi] = "--enable-muxer=avi,--disable-muxer=avi"
PACKAGECONFIG[encoder-mjpeg] = "--enable-encoder=mjpeg,--disable-encoder=mjpeg"
PACKAGECONFIG[protocol-file] = "--enable-protocol=file,--disable-protocol=file"
PACKAGECONFIG[disable-shared] = "--disable-shared,"
PACKAGECONFIG[enable-small] = "--enable-small,"
PACKAGECONFIG[enable-static] = "--enable-static,"
PACKAGECONFIG[disable-doc] = "--disable-doc,"
PACKAGECONFIG[disable-ffmpeg] = "--disable-ffmpeg,"
PACKAGECONFIG[disable-ffplay] = "--disable-ffplay,"
PACKAGECONFIG[disable-ffprobe] = "--disable-ffprobe,"

PACKAGECONFIG = "disable-everything \
                 muxer-avi encoder-mjpeg protocol-file \
                 disable-shared enable-small enable-static disable-doc \
                 disable-ffmpeg disable-ffplay disable-ffprobe \
                "

