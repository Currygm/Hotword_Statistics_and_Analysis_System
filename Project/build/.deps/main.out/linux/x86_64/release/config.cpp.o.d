{
    files = {
        "config.cpp"
    },
    depfiles_gcc = "config.o: config.cpp config.h cppjieba/Jieba.hpp  cppjieba/QuerySegment.hpp cppjieba/limonp/Logging.hpp  cppjieba/DictTrie.hpp cppjieba/limonp/StringUtil.hpp  cppjieba/limonp/StdExtension.hpp cppjieba/Unicode.hpp  cppjieba/limonp/LocalVector.hpp cppjieba/Trie.hpp  cppjieba/limonp/StdExtension.hpp cppjieba/SegmentBase.hpp  cppjieba/PreFilter.hpp cppjieba/FullSegment.hpp cppjieba/MixSegment.hpp  cppjieba/MPSegment.hpp cppjieba/SegmentTagged.hpp cppjieba/PosTagger.hpp  cppjieba/HMMSegment.hpp cppjieba/HMMModel.hpp  cppjieba/KeywordExtractor.hpp\
",
    values = {
        "/usr/bin/gcc",
        {
            "-m64",
            "-fvisibility=hidden",
            "-fvisibility-inlines-hidden",
            "-O2",
            "-std=c++11",
            "-Icppjieba",
            "-DNDEBUG"
        }
    }
}