{
    depfiles_gcc = "main.o: sources/main.cpp sources/../includefile/config.h  sources/../includefile/cppjieba/Jieba.hpp  sources/../includefile/cppjieba/QuerySegment.hpp  sources/../includefile/cppjieba/limonp/Logging.hpp  sources/../includefile/cppjieba/DictTrie.hpp  sources/../includefile/cppjieba/limonp/StringUtil.hpp  sources/../includefile/cppjieba/limonp/StdExtension.hpp  sources/../includefile/cppjieba/Unicode.hpp  sources/../includefile/cppjieba/limonp/LocalVector.hpp  sources/../includefile/cppjieba/Trie.hpp  sources/../includefile/cppjieba/limonp/StdExtension.hpp  sources/../includefile/cppjieba/SegmentBase.hpp  sources/../includefile/cppjieba/PreFilter.hpp  sources/../includefile/cppjieba/FullSegment.hpp  sources/../includefile/cppjieba/MixSegment.hpp  sources/../includefile/cppjieba/MPSegment.hpp  sources/../includefile/cppjieba/SegmentTagged.hpp  sources/../includefile/cppjieba/PosTagger.hpp  sources/../includefile/cppjieba/HMMSegment.hpp  sources/../includefile/cppjieba/HMMModel.hpp  sources/../includefile/cppjieba/KeywordExtractor.hpp\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O3",
            "-std=c++11",
            "-Iincludefile",
            "-DNDEBUG"
        }
    },
    files = {
        "sources/main.cpp"
    }
}