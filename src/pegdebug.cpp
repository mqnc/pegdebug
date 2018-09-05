
#include "peglib.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <streambuf>

using namespace peg;
using namespace std;

// helper for repeating output
struct repeat{
	repeat(const char* s_, std::size_t num_):s(s_), num(num_){}
	const char* s;
	std::size_t num;
};
inline std::ostream& operator <<(std::ostream& stream, const repeat& rep) {
	for(std::size_t i=0; i<rep.num; ++i){
		stream << rep.s;
	}
	return stream;
}

// read complete file into string
// https://stackoverflow.com/a/2602060/3825996
bool readFile(const string &path, string &txt){
	ifstream ifs(path.c_str());
	if(ifs.is_open()){
		txt = string((std::istreambuf_iterator<char>(ifs)),
		             (std::istreambuf_iterator<char>()));
		return true;
	}
	else{
		return false;
	}
}
// write string into file
bool writeFile(const string &path, const string &txt){
	ofstream ofs(path);
	if(ofs.is_open()){
		ofs << txt;
		ofs.close();
		return true;
	}
	else{
		return false;
	}
}

// https://stackoverflow.com/a/3418285/3825996
bool replace(string& str, const string& from, const string& to) {
	size_t start_pos = str.find(from);
	if(start_pos == string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if(from.empty())
		return;
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

int Main(const vector<string> &args) {

	if(args.size() != 4){
		cerr << "usage: " << args[0] << " mygrammar.peg mytext.txt myoutput.html\n";
		return EXIT_FAILURE;
	}

	#include "html.inc" // creates std::string html holding the html source

	bool ok = false;

	// read grammar file
	string grammar;
	ok = readFile(args[1], grammar);
	if(!ok){
		cerr << "error reading file " << args[1] << "\n";
		return EXIT_FAILURE;
	}
	replaceAll(grammar, "\r\n", "\n"); // in case we open a windows text file on linux

	// read text file to parse
	string text;
	ok = readFile(args[2], text);
	if(!ok){
		cerr << "error reading file " << args[2] << "\n";
		return EXIT_FAILURE;
	}
	replaceAll(text, "\r\n", "\n"); // in case we open a windows text file on linux

	// create parser
	parser parser;

	parser.log = [](size_t line, size_t col, const string& msg) {
		cerr << line << ":" << col << ": " << msg << "\n";
	};

	// load grammar
	ok = parser.load_grammar(grammar.c_str());
	if(!ok){
		cerr << "error loading grammar\n";
		return EXIT_FAILURE;
	}

	// pointer to text start (will be used to find current position in text)
	const char* pStart = text.c_str();

	// list all reduction rules
	const auto rules = parser.get_rule_names();

	// output string for tree view
	stringstream tree;

	struct substitution{
		const char* start; // where the substitution starts in the original text
		size_t len=0; // how long the substituted text is
		string insert=""; // the text that has to be inserted
	};

	// assign callbacks for enter, match and leave of each rule (html wrapping of the parsee)
	for(auto& rule:rules){

		parser[rule.c_str()].enter = [rule, &pStart, &tree](const char* s, size_t n, any& dt) {
			auto& indent = *dt.get<int*>();
			tree << repeat("  ", indent) << "<div title=\"" << rule << "\" data-pos=" << (s-pStart) << ">" << "\n";
			indent++;
		};

		parser[rule.c_str()] = [rule](const SemanticValues& sv, any&) {
			string result = sv.str();
			const char* start = sv.c_str();
			for(int i = sv.size()-1; i>=0; i--){
				auto sub = sv[i].get<substitution>();
				result = result.replace(sub.start-start, sub.len, sub.insert);
			}
			result = "\x01A_1" + rule + "\x01A_2" + to_string(start - sv.ss) + "\x01A_3" + result + "\x01A_4"; // use ascii SUB for later substitution
			return substitution{start, sv.length(), result};
		};

		parser[rule.c_str()].leave = [rule, &pStart, &tree](const char* s, size_t n, size_t matchlen, any& value, any& dt) {
			auto& indent = *dt.get<int*>();
			int match = success(matchlen)? matchlen : -1;
			tree << repeat("  ", indent) << "<span data-match=" << match << "></span>" << "\n";
			indent--;
			tree << repeat("  ", indent) << "</div>\n";
		};

	}
	parser.log = [](size_t ln, size_t col, const string& msg) {
		cout << "(" << ln << ":" << col << ") " << msg << "\n";
	};

	//parser.enable_packrat_parsing(); // enable packrat parsing

	// parse
	int indent = 0;
	any dt = &indent;

	substitution result;
	ok = parser.parse(text.c_str(), dt, result);

	if(ok){
		cout << "parsing successful\n";
	}
	else{
		cout << "parsing not successful\n";
	}

	// encode tree text for javascript string
	replaceAll(text, "\\", "\\\\");
	replaceAll(text, "\"", "\\\"");
	replaceAll(text, "\n", "\\n");
	replaceAll(text, "\t", "\\t");
	text = "\"" + text + "\"";

	// encode source text for html display
	string source = result.insert;
	replaceAll(source, "<", "&lt;");
	replaceAll(source, ">", "&gt;");
	replaceAll(source, "&", "&amp;");
	replaceAll(source, " ", "<i>&#x2423;</i>");
	replaceAll(source, "\t", "<i>&rarr;</i>&nbsp;&nbsp;&nbsp;");
	replaceAll(source, "\n", "<i>&ldsh;</i><br>\n");
	
	// now do substitutions that had to be immune to the substitutions before
	replaceAll(source, "\x01A_1", "<div title=\"");
	replaceAll(source, "\x01A_2", "\" data-pos=");
	replaceAll(source, "\x01A_3", ">");
	replaceAll(source, "\x01A_4", "</div>");

	// place generated texts in html
	replace(html, "TREE", tree.str());
	replace(html, "SOURCE", source);
	replace(html, "TEXT", text);

	ok = writeFile(args[3], html);
	if(!ok){
		cerr << "error writing file " << args[3] << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int c,char**v){return Main(vector<string>(v,c+v));}
