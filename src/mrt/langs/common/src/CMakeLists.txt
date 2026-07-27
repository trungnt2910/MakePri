makepri_add_library(
    makepri_langslib
    STATIC
    Bcp47Mrm.cpp
    Bcp47Parser.cpp
    Language.cpp
    Region.cpp
    SubtagTables.cpp
)
add_library(makepri_resources_langs ALIAS makepri_langslib)
