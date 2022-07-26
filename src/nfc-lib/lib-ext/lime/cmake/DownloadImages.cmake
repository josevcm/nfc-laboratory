########################################################################
## Download limesuite images upon configure
## Recommended for source-based build environments
########################################################################
if (NOT DOWNLOAD_IMAGES)
    return()
endif ()

message(STATUS "")
message(STATUS "######################################################")
message(STATUS "## DOWNLOAD_IMAGES=${DOWNLOAD_IMAGES} specified; syncing images...")
message(STATUS "######################################################")

#download support constants
set(SOURCE_URL "https://downloads.myriadrf.org/project/limesuite/${VERSION_MAJOR}.${VERSION_MINOR}")
set(TEMP_DEST "${CMAKE_CURRENT_BINARY_DIR}/images/${VERSION_MAJOR}.${VERSION_MINOR}")
set(INSTALL_DEST "share/LimeSuite/images/${VERSION_MAJOR}.${VERSION_MINOR}")
set(HREF_MATCHER "href=\\\"/project/limesuite/${VERSION_MAJOR}.${VERSION_MINOR}/([\\._A-Za-z_0-9-]+(.rpd|.rbf|.img))\\\"")

#conditional download when file is missing
function(DOWNLOAD_URL url file)
    if (EXISTS ${file})
        message(STATUS "Already downloaded ${url}")
    else ()
        message(STATUS "Downloading ${url}...")
        file(DOWNLOAD "${url}" "${file}")
    endif ()
endfunction(DOWNLOAD_URL)

#get a list of images from the url index
file(MAKE_DIRECTORY "${TEMP_DEST}")
DOWNLOAD_URL("${SOURCE_URL}" "${TEMP_DEST}/index.html")
file(READ "${TEMP_DEST}/index.html" index_html)
string(REGEX MATCHALL "${HREF_MATCHER}" matches "${index_html}")

#download each image in the index and create install rule
foreach (match ${matches})
    string(REGEX MATCHALL "${HREF_MATCHER}" __tmp "${match}")
    DOWNLOAD_URL("${SOURCE_URL}/${CMAKE_MATCH_1}" "${TEMP_DEST}/${CMAKE_MATCH_1}")
    install(FILES "${TEMP_DEST}/${CMAKE_MATCH_1}" DESTINATION ${INSTALL_DEST})
endforeach(match)
