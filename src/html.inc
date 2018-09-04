
// This is the dirtiest piece of code I have ever come up with.
// It allows to edit template.html as html and view it in the
// browser. It can then be included via #include into c++ source.
// The std::string html holding the html source is created.
//
// The included html file must begin with
//     <!--hack; html = R"do_not_remove(-->
// and end with
//     <!--)do_not_remove"; //-->

std::string html = "";
{
	char hack = 1;
	hack
	#include "template.html"
}

// remove "-->" from the beginning and "<!--" from the end
html = html.substr(3, html.length()-7);
