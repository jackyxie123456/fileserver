#include "HttpParser.h"
#include "gumbo.h"
#include <string>

static const char* find_title(const GumboNode* root) {
	//assert(root->type == GUMBO_NODE_ELEMENT);
	//assert(root->v.element.children.length >= 2);
	if (!root || root->v.element.children.length < 2)
		return "";

	const GumboVector* root_children = &root->v.element.children;
	GumboNode* head = NULL;
	for (int i = 0; i < root_children->length; ++i) {
		GumboNode* child = (GumboNode*)root_children->data[i];
		if (child->type == GUMBO_NODE_ELEMENT &&
			child->v.element.tag == GUMBO_TAG_HEAD) {
			head = child;
			break;
		}
	}
	//assert(head != NULL);
	if (NULL == head)
		return "";

	GumboVector* head_children = &head->v.element.children;
	if (NULL == head_children)
		return "";

	for (int i = 0; i < head_children->length; ++i) {
		GumboNode* child = (GumboNode*)head_children->data[i];
		if (child->type == GUMBO_NODE_ELEMENT &&
			child->v.element.tag == GUMBO_TAG_TITLE) {
			if (child->v.element.children.length != 1) {
				return "empty title";
			}
			GumboNode* title_text = (GumboNode *)child->v.element.children.data[0];
			if(title_text->type == GUMBO_NODE_TEXT || title_text->type == GUMBO_NODE_WHITESPACE)
				return title_text->v.text.text;
		}
	}
	return "";
}

std::string HttpParser::parse(const char* htmlFile) {
	GumboOutput* output = gumbo_parse(htmlFile);
	if (NULL == output)
		return "";//
	
	const char* strTitle = find_title(output->root);
	if (NULL != strTitle)
	{
		std::string result(strTitle);
		gumbo_destroy_output(&kGumboDefaultOptions, output);
		return result;
	}
	// Do stuff with output->root
	gumbo_destroy_output(&kGumboDefaultOptions, output);

	return "";
}

