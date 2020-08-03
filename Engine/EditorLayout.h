#pragma once
#include "IMGUI/imgui.h"

struct EditorLayout
{
	ImVec2 _objListPos;
	ImVec2 _objListSize;

	ImVec2 _previewPos;
	ImVec2 _previewSize;

	ImVec2 _fileBrowserPos;
	ImVec2 _fileBrowserSize;


	EditorLayout() {}


	EditorLayout(float w, float h)
	{
		_objListPos		= ImVec2(w * .8f, 0.);
		_objListSize	= ImVec2(w * .2f, h * .75f);

		_previewPos		= ImVec2(0.     , 0.);
		_previewSize	= ImVec2(w * .2, h * 0.75);

		_fileBrowserPos	= ImVec2(0., h * 0.75);
		_fileBrowserSize	= ImVec2(w , h * 0.25);
	}
};