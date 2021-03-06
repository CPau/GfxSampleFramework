//-----------------------------------------------------------------------------
// USER IMPLEMENTATION
// This file contains compile-time options for ImGui.
// Other options (memory allocation overrides, callbacks, etc.) can be set at runtime via the ImGuiIO structure - ImGui::GetIO().
//-----------------------------------------------------------------------------

#pragma once

#include <frm/core/def.h>
#include <frm/core/math.h>
#include <IconFontCppHeaders/IconsFontAwesome4.h>

//---- Define assertion handler. Defaults to calling assert().
#define IM_ASSERT(_EXPR)  APT_ASSERT(_EXPR)

//---- Define attributes of all API symbols declarations, e.g. for DLL under Windows.
//#define IMGUI_API __declspec( dllexport )
//#define IMGUI_API __declspec( dllimport )

//---- Include imgui_user.h at the end of imgui.h
//#define IMGUI_INCLUDE_IMGUI_USER_H

//---- Don't implement default handlers for Windows (so as not to link with OpenClipboard() and others Win32 functions)
//#define IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCS
//#define IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCS

//---- Don't implement help and test window functionality (ShowUserGuide()/ShowStyleEditor()/ShowTestWindow() methods will be empty)
//#define IMGUI_DISABLE_TEST_WINDOWS

//---- Don't define obsolete functions names
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

//---- Implement STB libraries in a namespace to avoid conflicts
#define IMGUI_STB_NAMESPACE     ImGuiStb

//---- Define constructor and implicit cast operators to convert back<>forth from your math types and ImVec2/ImVec4.

#define IM_VEC2_CLASS_EXTRA                                                 \
        ImVec2(const frm::vec2& f) { x = f.x; y = f.y; }                       \
        operator frm::vec2() const { return frm::vec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                 \
        ImVec4(const frm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }     \
        operator frm::vec4() const { return frm::vec4(x,y,z,w); }


//---- Tip: You can add extra functions within the ImGui:: namespace, here or in your own headers files.
//---- e.g. create variants of the ImGui::Value() helper for your low-level math types, or your own widgets/helpers.
/*
namespace ImGui
{
    void    Value(const char* prefix, const MyMatrix44& v, const char* float_format = NULL);
}
*/

#define IM_COLOR_ALPHA(_rgb8, _af) (((_rgb8) & ~IM_COL32_A_MASK) | ((ImU32)((_af) * 255.0f) << IM_COL32_A_SHIFT))
#define IM_COL32_RED      IM_COL32(255, 0,   0,   255)
#define IM_COL32_GREEN    IM_COL32(0,   255, 0,   255)
#define IM_COL32_BLUE     IM_COL32(0,   0,   255, 255)
#define IM_COL32_MAGENTA  IM_COL32(255, 0,   255, 255)
#define IM_COL32_YELLOW   IM_COL32(255, 255, 0,   255)
#define IM_COL32_CYAN     IM_COL32(0,   255, 255, 255)

namespace ImGui
{
	void Value(const char* prefix, const frm::vec2& v);
	void Value(const char* prefix, const frm::vec3& v);
	void Value(const char* prefix, const frm::vec4& v);
	void Value(const char* prefix, const frm::mat3& v);
	void Value(const char* prefix, const frm::mat4& v);

	bool ComboInt(const char* label, int* current_value, const char* items_separated_by_zeros, const int* item_values, int value_count);
	bool ComboFloat(const char* label, float* current_value, const char* items_separated_by_zeros, const float* item_values, int value_count);

	// Begin/end an invisible window at origin with the specified size (useful for drawing custom overlays).
	bool BeginInvisible(const char* name, frm::vec2 origin, frm::vec2 size, bool* p_open = NULL, int flags = 0);
	void EndInvisible();
}
