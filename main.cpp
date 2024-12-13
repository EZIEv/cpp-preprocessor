#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

bool IsFileInDir(const path& in_file, const path& target_file) {
    return filesystem::exists((filesystem::absolute(in_file)).parent_path() / target_file);
}

int FindFileInIncludeDirectories(const path& target_file, const vector<path>& include_directories) {
    for (size_t i = 0; i < include_directories.size(); ++i) {
        if (filesystem::exists((include_directories[i] / target_file))) {
            return i;
        }
    }

    return -1;
}

// напишите эту функцию
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    ifstream in(in_file);
    if (!in) {
        return false;
    }

    ofstream out(out_file, ios_base::app);

    static regex include_rel(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static regex include_abs(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");
    smatch m;

    size_t line_num = 1;
    string line;

    while (getline(in, line)) {
        if (regex_match(line, m, include_rel)) {
            path target_file = m[1].str();
            if (IsFileInDir(in_file, target_file)) {
                if (!Preprocess((filesystem::absolute(in_file)).parent_path() / target_file, filesystem::absolute(out_file), include_directories)) {
                    return false;
                }
            } else {
                int index = FindFileInIncludeDirectories(target_file, include_directories);
                if (index != -1) {
                    if (!Preprocess(include_directories[index] / target_file, filesystem::absolute(out_file), include_directories)) {
                        return false;
                    }
                } else {
                    cout << "unknown include file "s << target_file.string() << " at file "s << in_file.string() << " at line "s << line_num << endl;
                    return false;
                }
            }
        } else if (regex_match(line, m, include_abs)) {
            path target_file = m[1].str(); 
            int index = FindFileInIncludeDirectories(target_file, include_directories);
            if (index != -1) {
                if (!Preprocess(include_directories[index] / target_file, filesystem::absolute(out_file), include_directories)) {
                    return false;
                }
            } else {
                cout << "unknown include file "s << target_file.string() << " at file "s << in_file.string() << " at line "s << line_num << endl;
                return false;
            }
        } else {
            out << line << endl;
        }

        ++line_num;
    }
    
    in.close();
    out.close();

    return true;
}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                                  {"sources"_p / "include1"_p,"sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
}