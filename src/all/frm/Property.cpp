#include <frm/Property.h>

#include <apt/log.h>
#include <apt/memory.h>
#include <apt/FileSystem.h>
#include <apt/Json.h>

#include <EASTL/utility.h>
#include <imgui/imgui.h>

#include <new>


using namespace apt;

namespace frm {

/******************************************************************************

                                Property

******************************************************************************/

// PUBLIC

int Property::GetTypeSize(Type _type)
{
	switch (_type) {
		case Type_Bool:    return (int)sizeof(bool);
		case Type_Int:     return (int)sizeof(int);
		case Type_Float:   return (int)sizeof(float);
		case Type_String:  return (int)sizeof(StringBase);
		default:           return -1;
	};
}

Property::Property(
	const char* _name,
	Type        _type,
	int         _count,
	const void* _default,
	const void* _min,
	const void* _max,
	void*       storage_,
	const char* _displayName,
	Edit*       _edit
	)
{
	APT_ASSERT(_name); // must provide a name
	m_name.set(_name);
	m_displayName.set(_displayName ? _displayName : _name);
	m_type = _type;
	m_pfEdit = _edit;
	m_count = (uint8)_count;
	int sizeBytes = GetTypeSize(_type) * _count;

	m_default = (char*)APT_MALLOC_ALIGNED(sizeBytes, 16);
	m_min     = (char*)APT_MALLOC_ALIGNED(sizeBytes, 16);
	m_max     = (char*)APT_MALLOC_ALIGNED(sizeBytes, 16);
	if (storage_) {
		m_data = (char*)storage_;
		m_ownsData = false;
	} else {
		m_data = (char*)APT_MALLOC_ALIGNED(sizeBytes, 16);
		m_ownsData = true;
	}

	if (_type == Type_String) {
	 // call StringBase ctor
		if (m_ownsData) {
			new((apt::String<0>*)m_data) apt::String<0>();
		}
		new((apt::String<0>*)m_default) apt::String<0>();
		((StringBase*)m_default)->set((const char*)_default);
	} else {
		memcpy(m_default, _default, sizeBytes);
	}

 // min/max aren't arrays
	if (_min) {
		memcpy(m_min, _min, GetTypeSize(_type));
	}
	if (_max) {
		memcpy(m_max, _max, GetTypeSize(_type));
	}

	setDefault();
}

Property::~Property()
{
	if (m_ownsData) {
		APT_FREE_ALIGNED(m_data);
	}
	APT_FREE_ALIGNED(m_default);
	APT_FREE_ALIGNED(m_min);
	APT_FREE_ALIGNED(m_max);
}

Property::Property(Property&& _rhs)
{
	memset(this, 0, sizeof(Property));
	m_type = Type_Count;
	swap(*this, _rhs);
}
Property& Property::operator=(Property&& _rhs)
{
	if (this != &_rhs) {
		swap(*this, _rhs);
	}
	return _rhs;
}
void frm::swap(Property& _a_, Property& _b_)
{
	using eastl::swap;
	swap(_a_.m_data,        _b_.m_data);
	swap(_a_.m_default,     _b_.m_default);
	swap(_a_.m_min,         _b_.m_min);
	swap(_a_.m_max,         _b_.m_max);
	swap(_a_.m_type,        _b_.m_type);
	swap(_a_.m_count,       _b_.m_count);
	swap(_a_.m_ownsData,    _b_.m_ownsData);
	swap(_a_.m_name,        _b_.m_name);
	swap(_a_.m_displayName, _b_.m_displayName);
	swap(_a_.m_pfEdit,      _b_.m_pfEdit);	
}

bool Property::edit()
{
	const int kStrBufLen = 1024;
	bool ret = false;
	if (m_pfEdit) {
		ret = m_pfEdit(*this);

	} else {
		switch (m_type) {
			case Type_Bool:
				switch (m_count) {
					case 1:
						ret |= ImGui::Checkbox((const char*)m_displayName, (bool*)m_data);
						break;
					default: {
						String displayName;
						for (int i = 0; i < (int)m_count; ++i) {
							displayName.setf("%s[%d]", (const char*)m_displayName, i); 
							ret |= ImGui::Checkbox((const char*)displayName, (bool*)m_data);
						}
						break;
					}	
				};
				break;
			case Type_Int:
				switch (m_count) {
					case 1:
						ret |= ImGui::SliderInt((const char*)m_displayName, (int*)m_data, *((int*)m_min), *((int*)m_max));
						break;
					case 2:
						ret |= ImGui::SliderInt2((const char*)m_displayName, (int*)m_data, *((int*)m_min), *((int*)m_max));
						break;
					case 3:
						ret |= ImGui::SliderInt3((const char*)m_displayName, (int*)m_data, *((int*)m_min), *((int*)m_max));
						break;
					case 4:
						ret |= ImGui::SliderInt4((const char*)m_displayName, (int*)m_data, *((int*)m_min), *((int*)m_max));
						break;
					default: {
						APT_ASSERT(false); // \todo arbitrary arrays, see bool
						break;
					}
				};
				break;
			case Type_Float:
				switch (m_count) {
					case 1:
						ret |= ImGui::SliderFloat((const char*)m_displayName, (float*)m_data, *((float*)m_min), *((float*)m_max));
						break;
					case 2:
						ret |= ImGui::SliderFloat2((const char*)m_displayName, (float*)m_data, *((float*)m_min), *((float*)m_max));
						break;
					case 3:
						ret |= ImGui::SliderFloat3((const char*)m_displayName, (float*)m_data, *((float*)m_min), *((float*)m_max));
						break;
					case 4:
						ret |= ImGui::SliderFloat4((const char*)m_displayName, (float*)m_data, *((float*)m_min), *((float*)m_max));
						break;
					default: {
						APT_ASSERT(false); // \todo arbitrary arrays, see bool
						break;
					}
				};
				break;
			case Type_String: {
				char buf[kStrBufLen];
				switch (m_count) {
					case 1: {
						StringBase& str = (StringBase&)*m_data;
						APT_ASSERT(str.getCapacity() < kStrBufLen);
						memcpy(buf, (const char*)str, str.getLength() + 1);
						if (ImGui::InputText((const char*)m_displayName, buf, kStrBufLen)) {
							str.set(buf);
							ret = true;
						}
						break;
					}
					default: {
						String displayName;
						for (int i = 0; i < (int)m_count; ++i) {
							displayName.setf("%s[%d]", (const char*)m_displayName, i); 
						 	StringBase& str = (StringBase&)*m_data;
							APT_ASSERT(str.getCapacity() < kStrBufLen);
							if (ImGui::InputText((const char*)displayName, buf, kStrBufLen)) {
								str.set(buf);
								ret = true;
							}
						}
						break;
					};
				};
				break;
			default:
				APT_ASSERT(false);
			}
		};
	}

	if (ImGui::GetIO().MouseClicked[1] && ImGui::IsItemHovered()) {
		setDefault();
	}
	return ret;
}

void Property::setDefault()
{
	if (getType() == Type_String) {
		((StringBase*)m_data)->set(((StringBase*)m_default)->c_str());
	} else {
		memcpy(m_data, m_default, GetTypeSize(getType()) * getCount());
	}
}

bool frm::Serialize(SerializerJson& _serializer_, Property& _prop_)
{
	bool ret = true;
	uint count = _prop_.m_count;
	if (count > 1) {
		if (_serializer_.beginArray(count, (const char*)_prop_.m_name)) {
			for (uint i = 0; i < count; ++i) {
				switch (_prop_.m_type) {
					case Property::Type_Bool:   ret &= Serialize(_serializer_, ((bool*)_prop_.m_data)[i]); break;
					case Property::Type_Int:    ret &= Serialize(_serializer_, ((int*)_prop_.m_data)[i]); break;
					case Property::Type_Float:  ret &= Serialize(_serializer_, ((float*)_prop_.m_data)[i]); break;
					case Property::Type_String: ret &= Serialize(_serializer_, ((StringBase*)_prop_.m_data)[i]); break;
					default:                    ret = false; APT_ASSERT(false);
				}
			}
			_prop_.m_count = (uint8)count;
			_serializer_.endArray();
		} else {
			ret = false;
		}
	} else {
		switch (_prop_.m_type) {
			case Property::Type_Bool:   ret &= Serialize(_serializer_, *((bool*)_prop_.m_data),       (const char*)_prop_.m_name); break;
			case Property::Type_Int:    ret &= Serialize(_serializer_, *((int*)_prop_.m_data),        (const char*)_prop_.m_name); break;
			case Property::Type_Float:  ret &= Serialize(_serializer_, *((float*)_prop_.m_data),      (const char*)_prop_.m_name); break;
			case Property::Type_String: ret &= Serialize(_serializer_, *((StringBase*)_prop_.m_data), (const char*)_prop_.m_name); break;
			default:                    ret = false; APT_ASSERT(false);
		}
	}
	return ret;
}


/******************************************************************************

                              PropertyGroup

******************************************************************************/

static bool EditColor(Property& _prop)
{
	bool ret = false;
	switch (_prop.getCount()) {
	case 3:
		ret = ImGui::ColorEdit3(_prop.getDisplayName(), (float*)_prop.getData());
		break;
	case 4:
		ret = ImGui::ColorEdit4(_prop.getDisplayName(), (float*)_prop.getData());
		break;
	default:
		APT_ASSERT(false);
		break;
	};
	return ret;
}

static bool EditPath(Property& _prop)
{
	bool ret = false;
	PathStr& pth = *((PathStr*)_prop.getData());
	if (ImGui::Button(_prop.getDisplayName())) {
		if (FileSystem::PlatformSelect(pth)) {
			pth = FileSystem::MakeRelative((const char*)pth);
			ret = true;
		}
	}
	ImGui::SameLine();
	ImGui::Text(ICON_FA_FLOPPY_O "  \"%s\"", (const char*)pth);
	return ret;
}

// PUBLIC

PropertyGroup::PropertyGroup(const char* _name)
	: m_name(_name)
{
}

PropertyGroup::~PropertyGroup()
{
	for (auto& it : m_props) {
		delete it.second;
	}
	m_props.clear();
}

PropertyGroup::PropertyGroup(PropertyGroup&& _rhs)
{
	using eastl::swap;
	swap(m_name,  _rhs.m_name);
	swap(m_props, _rhs.m_props);
}
PropertyGroup& PropertyGroup::operator=(PropertyGroup&& _rhs)
{
	using eastl::swap;
	if (this != &_rhs) {
		swap(m_name,  _rhs.m_name);
		swap(m_props, _rhs.m_props);
	}
	return *this;
}
void frm::swap(PropertyGroup& _a_, PropertyGroup& _b_)
{
	using eastl::swap;
	swap(_a_.m_name,  _b_.m_name);
	swap(_a_.m_props, _b_.m_props);
}

bool* PropertyGroup::addBool(const char* _name, bool _default, bool* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Bool, 1, &_default, nullptr, nullptr, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asBool();
}
int* PropertyGroup::addInt(const char* _name, int _default, int _min, int _max, int* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Int, 1, &_default, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asInt();
}
ivec2* PropertyGroup::addInt2(const char* _name, const ivec2& _default, int _min, int _max, ivec2* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Int, 2, &_default.x, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asInt2();
}
ivec3* PropertyGroup::addInt3(const char* _name, const ivec3& _default, int _min, int _max, ivec3* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Int, 3, &_default.x, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asInt3();
}
ivec4* PropertyGroup::addInt4(const char* _name, const ivec4& _default, int _min, int _max, ivec4* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Int, 4, &_default.x, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asInt4();
}
float* PropertyGroup::addFloat(const char* _name, float _default, float _min, float _max, float* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Float, 1, &_default, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asFloat();
}
vec2* PropertyGroup::addFloat2(const char* _name, const vec2& _default, float _min, float _max, vec2* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Float, 2, &_default.x, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asFloat2();
}
vec3* PropertyGroup::addFloat3(const char* _name, const vec3& _default, float _min, float _max, vec3* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Float, 3, &_default.x, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asFloat3();
}
vec4* PropertyGroup::addFloat4(const char* _name, const vec4& _default, float _min, float _max, vec4* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Float, 4, &_default.x, &_min, &_max, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asFloat4();
}
vec3* PropertyGroup::addRgb(const char* _name, const vec3& _default, float _min, float _max, vec3* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Float, 3, &_default.x, &_min, &_max, storage_, _displayName, &EditColor);
	m_props[StringHash(_name)] = ret;
	return ret->asFloat3();
}
vec4* PropertyGroup::addRgba(const char* _name, const vec4& _default, float _min, float _max, vec4* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_Float, 4, &_default.x, &_min, &_max, storage_, _displayName, &EditColor);
	m_props[StringHash(_name)] = ret;
	return ret->asFloat4();
}
StringBase* PropertyGroup::addString(const char* _name, const char* _default, StringBase* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_String, 1, _default, nullptr, nullptr, storage_, _displayName, nullptr);
	m_props[StringHash(_name)] = ret;
	return ret->asString();
}
StringBase* PropertyGroup::addPath(const char* _name, const char* _default, StringBase* storage_, const char* _displayName)
{
	Property* ret = new Property(_name, Property::Type_String, 1, _default, nullptr, nullptr, storage_, _displayName, &EditPath);
	m_props[StringHash(_name)] = ret;
	return ret->asString();
}

Property* PropertyGroup::find(StringHash _nameHash)
{
	auto ret = m_props.find(_nameHash);
	if (ret != m_props.end()) {
		return ret->second;
	}
	return nullptr;
}

bool PropertyGroup::edit(bool _showHidden)
{
	bool ret = false;
	for (auto& it : m_props) {
		Property& prop = *it.second;
		if (_showHidden || prop.getName()[0] != '#') {
			ret |= prop.edit();
		}
	}
	return ret;
}

bool frm::Serialize(SerializerJson& _serializer_, PropertyGroup& _propGroup_)
{
	if (_serializer_.beginObject((const char*)_propGroup_.m_name)) {
		bool ret = true;
		for (auto& it : _propGroup_.m_props) {
			ret &= Serialize(_serializer_, *it.second);
		}
		_serializer_.endObject();
		return ret;
	}
	return false;
}

/******************************************************************************

                              Properties

******************************************************************************/

// PUBLIC

Properties::~Properties()
{
	for (auto& it : m_groups) {
		delete it.second;
	}
	m_groups.clear();
}


PropertyGroup& Properties::addGroup(const char* _name)
{
	PropertyGroup* group = findGroup(_name);
	if (group) {
		return *group;
	}
	PropertyGroup* ret = new PropertyGroup(_name);
	m_groups[StringHash(_name)] = ret;
	return *ret;
}

Property* Properties::findProperty(StringHash _nameHash)
{
	for (auto& group : m_groups) {
		Property* prop = group.second->find(_nameHash);
		if (prop) {
			return prop;
		}
	}
	return nullptr;
}

PropertyGroup* Properties::findGroup(StringHash _nameHash)
{
	auto ret = m_groups.find(_nameHash);
	if (ret != m_groups.end()) {
		return ret->second;
	}
	return nullptr;	
}

bool Properties::edit(bool _showHidden)
{
	bool ret = false;
	for (auto& it: m_groups) {
		PropertyGroup& group = *it.second;
		if (ImGui::TreeNode(group.getName())) {
			ret |= group.edit(_showHidden);
			ImGui::TreePop();
		}
	}
	return ret;
}

bool frm::Serialize(SerializerJson& _serializer_, Properties& _props_)
{
	bool ret = true;
	for (auto& it : _props_.m_groups) {
		ret &= Serialize(_serializer_, *it.second);
	}
	return true;
}

namespace refactor {


/******************************************************************************

                              Properties

******************************************************************************/

// PUBLIC

const char* Properties::GetTypeStr(Type _type)
{
	switch (_type) {
		case Type_Bool:   return "Bool";
		case Type_Int:    return "Int";
		case Type_Float:  return "Float";
		case Type_String: return "String";
		default:          return "Unknown Type";
	};
}

int Properties::GetTypeSizeBytes(Type _type)
{
	switch (_type) {
		case Type_Bool:   return (int)sizeof(bool);
		case Type_Int:    return (int)sizeof(int);
		case Type_Float:  return (int)sizeof(float);
		case Type_String: return (int)sizeof(StringBase);
		default:          return 0;
	};
}

#define Properties_GetTypeCount(_T, _retType, _retCount) \
	template <> Properties::Type Properties::GetType<_T>()  { return _retType; } \
	template <> Properties::Type Properties::GetCount<_T>() { return _retCount; }

Properties_GetTypeCount(bool,       Properties::Type_Bool,   1);
Properties_GetTypeCount(bvec2,      Properties::Type_Bool,   2);
Properties_GetTypeCount(bvec3,      Properties::Type_Bool,   3);
Properties_GetTypeCount(bvec4,      Properties::Type_Bool,   4);

Properties_GetTypeCount(int,        Properties::Type_Int,    1);
Properties_GetTypeCount(ivec2,      Properties::Type_Int,    2);
Properties_GetTypeCount(ivec3,      Properties::Type_Int,    3);
Properties_GetTypeCount(ivec4,      Properties::Type_Int,    4);

Properties_GetTypeCount(float,      Properties::Type_Float,  1);
Properties_GetTypeCount(vec2,       Properties::Type_Float,  2);
Properties_GetTypeCount(vec3,       Properties::Type_Float,  3);
Properties_GetTypeCount(vec4,       Properties::Type_Float,  4);

Properties_GetTypeCount(StringBase, Properties::Type_String, 1);


bool Properties::DefaultEditFunc(Property& _prop)
{
	constexpr const int kStrBufLen = 512;
	bool ret = false;
	void* data = _prop.getStorage();
	switch (_prop.m_type) {
		case Type_Bool:
			switch (_prop.m_count) {
				case 1:
					ret |= ImGui::Checkbox((const char*)_prop.m_displayName, (bool*)data);
					break;
				default: {
					String<64> displayName;
					for (int i = 0; i < _prop.m_count; ++i) {
						displayName.setf("%s[%d]", (const char*)_prop.m_displayName, i); 
						ret |= ImGui::Checkbox((const char*)displayName, (bool*)data);
					}
					break;
				}	
			};
			break;
		case Type_Int:
			switch (_prop.m_count) {
				case 1:
					ret |= ImGui::SliderInt((const char*)_prop.m_displayName, (int*)data, *((int*)_prop.m_min), *((int*)_prop.m_max));
					break;
				case 2:
					ret |= ImGui::SliderInt2((const char*)_prop.m_displayName, (int*)data, *((int*)_prop.m_min), *((int*)_prop.m_max));
					break;
				case 3:
					ret |= ImGui::SliderInt3((const char*)_prop.m_displayName, (int*)data, *((int*)_prop.m_min), *((int*)_prop.m_max));
					break;
				case 4:
					ret |= ImGui::SliderInt4((const char*)_prop.m_displayName, (int*)data, *((int*)_prop.m_min), *((int*)_prop.m_max));
					break;
				default: {
					APT_ASSERT(false); // \todo arbitrary arrays, see bool
					break;
				}
			};
			break;
		case Type_Float:
			switch (_prop.m_count) {
				case 1:
					ret |= ImGui::SliderFloat((const char*)_prop.m_displayName, (float*)data, *((float*)_prop.m_min), *((float*)_prop.m_max));
					break;
				case 2:
					ret |= ImGui::SliderFloat2((const char*)_prop.m_displayName, (float*)data, *((float*)_prop.m_min), *((float*)_prop.m_max));
					break;
				case 3:
					ret |= ImGui::SliderFloat3((const char*)_prop.m_displayName, (float*)data, *((float*)_prop.m_min), *((float*)_prop.m_max));
					break;
				case 4:
					ret |= ImGui::SliderFloat4((const char*)_prop.m_displayName, (float*)data, *((float*)_prop.m_min), *((float*)_prop.m_max));
					break;
				default: {
					APT_ASSERT(false); // \todo arbitrary arrays, see bool
					break;
				}
			};
			break;
		case Type_String: {
			char buf[kStrBufLen];
			switch (_prop.m_count) {
				case 1: {
					apt::StringBase& str = *(apt::StringBase*)data;
					APT_ASSERT(str.getCapacity() < kStrBufLen);
					memcpy(buf, (const char*)str, str.getLength() + 1);
					if (ImGui::InputText((const char*)_prop.m_displayName, buf, kStrBufLen)) {
						str.set(buf);
						ret = true;
					}
					break;
				}
				default: {
					String<64> displayName;
					for (int i = 0; i < _prop.m_count; ++i) {
						displayName.setf("%s[%d]", (const char*)_prop.m_displayName, i); 
					 	apt::StringBase& str = *(apt::StringBase*)data;
						APT_ASSERT(str.getCapacity() < kStrBufLen);
						if (ImGui::InputText((const char*)displayName, buf, kStrBufLen)) {
							str.set(buf);
							ret = true;
						}
					}
					break;
				};
			};
			break;
		default:
			APT_ASSERT(false);
			break;
		}
	};
	return ret;
}

bool Properties::ColorEditFunc(Property& _prop_)
{
	APT_ASSERT(_prop_.getType() == Type_Float);

	float* data = (float*)_prop_.getStorage();
	bool ret = false;
	if (_prop_.getCount() == 3) {
		ret = ImGui::ColorEdit3(_prop_.getName(), data);
	} else {
		ret = ImGui::ColorEdit4(_prop_.getName(), data);
	}
	return ret;
}

bool Properties::PathEditFunc(Property& _prop_)
{
	bool ret = false;
	PathStr& pth = *((PathStr*)_prop_.getStorage());
	if (ImGui::Button(_prop_.getDisplayName())) {
		if (FileSystem::PlatformSelect(pth)) {
			pth = FileSystem::MakeRelative((const char*)pth);
			ret = true;
		}
	}
	ImGui::SameLine();
	ImGui::Text("\"%s\"", (const char*)pth);
	return ret;
}

void Properties::DefaultDisplayFunc(const Property& _prop)
{
	ImGui::Text("%s: ", (const char*)_prop.m_displayName);
	void* data = _prop.getStorage();
	switch (_prop.m_type) {
		case Type_Bool:
			for (int i = 0; i < _prop.m_count; ++i) {
				ImGui::SameLine();
				ImGui::Text("%d ", (int)((bool*)data)[i]);
			}
			break;
		case Type_Int:
			for (int i = 0; i < _prop.m_count; ++i) {
				ImGui::SameLine();
				ImGui::Text("%d ", ((int*)data)[i]);
			}
			break;
		case Type_Float:
			for (int i = 0; i < _prop.m_count; ++i) {
				ImGui::SameLine();
				ImGui::Text("%+07.3f ", ((float*)data)[i]);
			}
			break;
		case Type_String:
			for (int i = 0; i < _prop.m_count; ++i) {
				ImGui::Text("%s", ((apt::StringBase*)data)[i].c_str());
			}
			break;
		default:
			APT_ASSERT(false);
			break;
	};
}

void Properties::ColorDisplayFunc(const Property& _prop)
{
	APT_ASSERT(_prop.getType() == Type_Float);

	ImGui::Text("%s: ", _prop.getName());
	ImGui::SameLine();
	const float* data = (float*)_prop.getStorage();
	ImGui::ColorButton(_prop.getName(), ImVec4(data[0], data[1], data[2], data[3]));
}

Properties* Properties::GetDefault()
{
	static Properties s_default;
	return &s_default;
}

Property* Properties::AddColor(const char* _name, const vec3& _default, vec3* _storage, const char* _displayName)
{
	auto ret = Add(_name, _default, _storage, _displayName);
	ret->setEditFunc(ColorEditFunc); 
	return ret;
}

Property* Properties::AddColor(const char* _name, const vec4& _default, vec4* _storage, const char* _displayName)
{
	auto ret = Add(_name, _default, _storage, _displayName);
	ret->setEditFunc(ColorEditFunc);
	return ret;
}

Property* Properties::AddPath(const char* _name, const PathStr& _default, PathStr* _storage, const char* _displayName)
{
	auto ret = Add(_name, (const StringBase&)_default, (StringBase*)_storage, _displayName);
	ret->setEditFunc(PathEditFunc);
	return ret;
}

void Properties::InvalidateStorage(const char* _propName, const char* _groupName)
{
	auto prop = Find(_propName, _groupName);
	if (prop) {
		prop->setExternalStorage(nullptr);
	}
}

Properties* Properties::Create()
{
	return APT_NEW(Properties());
}

void Properties::Destroy(Properties*& _properties_)
{
	APT_DELETE(_properties_);
	_properties_ = nullptr;
}


bool frm::refactor::Serialize(SerializerJson& _serializer_, Properties& _props_)
{
	bool ret = true;

	if (_serializer_.getMode() == SerializerJson::Mode_Read) {
		Json* json = _serializer_.getJson();
		while (json->next()) { // for each group
			json->enterObject();
			_props_.pushGroup(json->getName());
				while (json->next()) { // for each property
					switch (json->getType()) {
					};
				}
			_props_.popGroup();
			json->leaveObject();
		}

	} else {
		for (auto& groupIt : _props_.m_groups) {
			auto  groupHash = groupIt.first;
			auto& group     = *groupIt.second;
			
			ret &= _serializer_.beginObject(_props_.m_groupNames[groupHash].c_str());
				for (auto& propIt : group) {
					auto& prop = *propIt.second;
					void* data = prop.getStorage();
					if (prop.getCount() == 1) {
						switch (prop.getType()) {
							case Properties::Type_Bool:    _serializer_.value(*((bool*)data),       prop.getName()); break;
							case Properties::Type_Int:     _serializer_.value(*((int*)data),        prop.getName()); break;
							case Properties::Type_Float:   _serializer_.value(*((float*)data),      prop.getName()); break;
							case Properties::Type_String:  _serializer_.value(*((StringBase*)data), prop.getName()); break;
							default:                       APT_ASSERT(false); break;
						};
					} else {
						uint count = (uint)prop.getCount();
						ret &= _serializer_.beginArray(count, prop.getName());
							for (uint i = 0; i < count; ++i) {
								switch (prop.getType()) {
									case Properties::Type_Bool:    _serializer_.value(((bool*)data)[i]); break;
									case Properties::Type_Int:     _serializer_.value(((int*)data)[i]); break;
									case Properties::Type_Float:   _serializer_.value(((float*)data)[i]); break;
									case Properties::Type_String:  _serializer_.value(((StringBase*)data)[i]); break;
									default:                       APT_ASSERT(false); break;
								};
							}
						_serializer_.endArray();
					}
				}
			_serializer_.endObject();
		}
	}

	return ret;
}


// PRIVATE

Properties* Properties::s_current = Properties::GetDefault();

Properties::Properties()
{
	m_groupStack.push_back(newGroup(kDefaultGroupName));
}
Properties::~Properties()
{
	while (!m_groups.empty()) {
		auto& group = *m_groups.back().second;
		while (!group.empty()) {
			APT_DELETE(group.back().second);
			group.pop_back();
		}
		APT_DELETE(&group);
		m_groups.pop_back();
	}
	m_groupStack.clear();
}

Property* Properties::add(const char* _name, Type _type, int _count, const void* _default, const void* _min, const void* _max, void* _storage, const char* _displayName)
{
	auto group = m_groupStack.back();
	APT_STRICT_ASSERT(group);
	auto nameHash = apt::StringHash(_name);
	auto it = group->find(nameHash);
	if (it != group->end()) {
	 // already exists
		auto prop = it->second;
		if (_type != prop->getType()) {
			APT_LOG_ERR("Properties: '%s' type mismatch (new: %s, existing: %s)", _name, GetTypeStr(_type), GetTypeStr(prop->getType()));
			APT_ASSERT(false);
		}
		if (_count != prop->getCount()) {
			APT_LOG_ERR("Properties: '%s' count mismatch (new: %d, existing: %d", _name, _count, prop->getCount());
			APT_ASSERT(false);
		}
		prop->setDefault(&_default);
		prop->setMin(&_min);
		prop->setMax(&_max);
		prop->setExternalStorage(_storage);
		prop->setDisplayName(_displayName);
		return prop;
	} else {
	 // doesn't exist, add new
		auto prop = APT_NEW(refactor::Property(_name, _displayName, _type, _count, _storage, _default, _min, _max));
		return prop;			
	}
}

Property* Properties::find(const char* _propName, const char* _groupName)
{
	APT_STRICT_ASSERT(_propName);

	const StringHash propHash  = StringHash(_propName);
	const StringHash groupHash = _groupName ? StringHash(_groupName) : StringHash::kInvalidHash;

 // resolve first group to search
	Group* group = m_groupStack.back();
	if (groupHash != StringHash::kInvalidHash) {
		auto found = m_groups.find(groupHash);
		if (found != m_groups.end()) {
			group = found->second;
		}
	}

 // find the prop
	auto ret = findInGroup(propHash, group);
	if (!ret && groupHash == StringHash::kInvalidHash) {
	 // if not found and no group specified, search all groups
		for (auto& g : m_groups) {
			if (g.second != group) {
				ret = findInGroup(propHash, g.second);
				if (ret) {
					break;
				}
			}
		}
	}

	if (!ret) {
		APT_LOG_ERR("Properties: '%s%s%s' not found.", _groupName ? _groupName : "", _groupName ? "/" : "", _propName);
	}

	return ret;
}

void Properties::pushGroup(const char* _group)
{
	APT_STRICT_ASSERT(_group);
	const StringHash groupHash = StringHash(_group);

}
void Properties::popGroup()
{
	APT_ASSERT(m_groupStack.size() > 1); // can't pop the default group
	if (m_groupStack.size() > 1) {
		m_groupStack.pop_back();
	}
}

Properties::Group* Properties::newGroup(const char* _groupName)
{
	StringHash groupHash = StringHash(_groupName);
	APT_ASSERT(m_groups.find(groupHash) == m_groups.end()); // group already exists

	Group* ret = APT_NEW(Group());
	m_groups[groupHash]     = ret;
	m_groupNames[groupHash] = _groupName;
	return ret;
}

refactor::Property* Properties::findInGroup(StringHash _propName, const Group* _group) const
{
	if (_propName == StringHash::kInvalidHash || !_group) {
		return nullptr;
	}
	auto ret = _group->find(_propName);
	if (ret != _group->end()) {
		return ret->second;
	}
	return nullptr; 
}

/******************************************************************************

                                Property

******************************************************************************/

// PUBLIC

void Property::reset()
{
	copy(m_storageInternal, m_default);
	if (m_storageExternal) {
		copy(m_storageExternal, m_default);
	}
}

void Property::setDefault(void* _default)
{
	copy(m_default, _default);
}

void Property::setMin(void* _min)
{
	copy(m_min, _min);
}

void Property::setMax(void* _max)
{
	copy(m_max, _max);
}

int Property::getSizeBytes() const
{
	return Properties::GetTypeSizeBytes(m_type) * m_count;
}

void Property::setExternalStorage(void* _storage_)
{
	if (_storage_) {
	 // setting external storage, copy current value from internal storage
		m_storageExternal = (char*)_storage_;
		copy(m_storageExternal, m_storageInternal);
	} else {
	 // invalidating external storage, copy current to internal storage
		copy(m_storageInternal, m_storageExternal);
		m_storageExternal = nullptr;
	}	
}

// PRIVATE

Property::Property(
	const char* _name,
	const char* _displayName,
	Type        _type,
	int         _count,
	void*       _storageExternal,
	const void* _default,
	const void* _min,
	const void* _max
	)
	: m_name(_name)
	, m_displayName(_displayName ? _displayName : _name)
	, m_type(_type)
	, m_count(_count)
	, m_storageExternal((char*)_storageExternal)
{
	const int sizeBytes = getSizeBytes();

	APT_ASSERT(_default); // default must not be 0
	m_default = (char*)APT_MALLOC(getSizeBytes());
	copy(m_default, _default);

	m_storageInternal = (char*)APT_MALLOC(getSizeBytes());
	copy(m_storageInternal, _default);

	if (m_storageExternal) {
		copy(m_storageExternal, _default);
	}

	if (_min) {
		m_min = (char*)APT_MALLOC(getSizeBytes());
		copy(m_min, _min);
	}
	if (_max) {
		m_max = (char*)APT_MALLOC(getSizeBytes());
		copy(m_max, _max);
	}
}

Property::~Property()
{
	if (m_storageExternal) {
		APT_LOG_ERR("Properties: '%s' external storage was not invalidated.");
	}
	APT_FREE(m_storageInternal);
	APT_FREE(m_default);
	APT_FREE(m_min);
	APT_FREE(m_max);
}

void Property::copy(void* dst_, const void* _src)
{
	APT_ASSERT(dst_ && _src);
	memcpy(dst_, _src, getSizeBytes());
}

} // namespace refactor

} // namespace frm