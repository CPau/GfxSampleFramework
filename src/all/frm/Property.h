#pragma once

#include <frm/def.h>
#include <frm/math.h>

#include <apt/String.h>
#include <apt/StringHash.h>

#include <EASTL/vector_map.h>

namespace frm {

////////////////////////////////////////////////////////////////////////////////
// Property
////////////////////////////////////////////////////////////////////////////////
class Property
{
public:
	typedef bool (Edit)(Property& _prop); // edit func, return true if the value changed
	typedef apt::StringBase StringBase;
	typedef apt::StringHash StringHash;

	enum Type : uint8
	{
		Type_Bool,
		Type_Int,
		Type_Float,
		Type_String,

		Type_Count
	};

	static int GetTypeSize(Type _type);

	Property(
		const char* _name,
		Type        _type,
		int         _count,
		const void* _default,
		const void* _min,
		const void* _max,
		void*       storage_,
		const char* _displayName,
		Edit*       _edit
		);

	~Property();

	Property(Property&& _rhs);
	Property& operator=(Property&& _rhs);
	friend void swap(Property& _a_, Property& _b_);

	bool        edit();
	void        setDefault();
	friend bool Serialize(apt::SerializerJson& _serializer_, Property& _prop_);

	bool*       asBool()               { APT_ASSERT(getType() == Type_Bool);   return (bool*)getData();       }
	int*        asInt()                { APT_ASSERT(getType() == Type_Int);    return (int*)getData();        }
	ivec2*      asInt2()               { APT_ASSERT(getType() == Type_Int);    return (ivec2*)getData();      }
	ivec3*      asInt3()               { APT_ASSERT(getType() == Type_Int);    return (ivec3*)getData();      }
	ivec4*      asInt4()               { APT_ASSERT(getType() == Type_Int);    return (ivec4*)getData();      }
	float*      asFloat()              { APT_ASSERT(getType() == Type_Float);  return (float*)getData();      }
	vec2*       asFloat2()             { APT_ASSERT(getType() == Type_Float);  return (vec2*)getData();       }
	vec3*       asFloat3()             { APT_ASSERT(getType() == Type_Float);  return (vec3*)getData();       }
	vec4*       asFloat4()             { APT_ASSERT(getType() == Type_Float);  return (vec4*)getData();       }
	vec3*       asRgb()                { APT_ASSERT(getType() == Type_Float);  return (vec3*)getData();       }
	vec4*       asRgba()               { APT_ASSERT(getType() == Type_Float);  return (vec4*)getData();       }
	StringBase* asString()             { APT_ASSERT(getType() == Type_String); return (StringBase*)getData(); }
	StringBase* asPath()               { APT_ASSERT(getType() == Type_String); return (StringBase*)getData(); }

	void*       getData() const        { return m_data; }
	Type        getType() const        { return m_type; }
	int         getCount() const       { return m_count; }
	const char* getName() const        { return (const char*)m_name; }
	const char* getDisplayName() const { return (const char*)m_displayName; }

private:
	typedef apt::String<32> String;

	char*  m_data;
	char*  m_default;
	char*  m_min;
	char*  m_max;
	Type   m_type;
	uint8  m_count;
	bool   m_ownsData;
	String m_name;
	String m_displayName;
	Edit*  m_pfEdit;

};


////////////////////////////////////////////////////////////////////////////////
// PropertyGroup
////////////////////////////////////////////////////////////////////////////////
class PropertyGroup: private apt::non_copyable<PropertyGroup>
{
public:
	typedef apt::StringBase StringBase;
	typedef apt::StringHash StringHash;

	PropertyGroup(const char* _name);
	~PropertyGroup();

	PropertyGroup(PropertyGroup&& _rhs);
	PropertyGroup& operator=(PropertyGroup&& _rhs);
	friend void swap(PropertyGroup& _a_, PropertyGroup& _b_);

	bool*       addBool  (const char* _name, bool         _default,                         bool*       storage_ = nullptr, const char* _displayName = nullptr);
	int*        addInt   (const char* _name, int          _default, int   _min, int   _max, int*        storage_ = nullptr, const char* _displayName = nullptr);
	ivec2*      addInt2  (const char* _name, const ivec2& _default, int   _min, int   _max, ivec2*      storage_ = nullptr, const char* _displayName = nullptr);
	ivec3*      addInt3  (const char* _name, const ivec3& _default, int   _min, int   _max, ivec3*      storage_ = nullptr, const char* _displayName = nullptr);
	ivec4*      addInt4  (const char* _name, const ivec4& _default, int   _min, int   _max, ivec4*      storage_ = nullptr, const char* _displayName = nullptr);
	float*      addFloat (const char* _name, float        _default, float _min, float _max, float*      storage_ = nullptr, const char* _displayName = nullptr);
	vec2*       addFloat2(const char* _name, const vec2&  _default, float _min, float _max, vec2*       storage_ = nullptr, const char* _displayName = nullptr);
	vec3*       addFloat3(const char* _name, const vec3&  _default, float _min, float _max, vec3*       storage_ = nullptr, const char* _displayName = nullptr);
	vec4*       addFloat4(const char* _name, const vec4&  _default, float _min, float _max, vec4*       storage_ = nullptr, const char* _displayName = nullptr);
	vec3*       addRgb   (const char* _name, const vec3&  _default, float _min, float _max, vec3*       storage_ = nullptr, const char* _displayName = nullptr);
	vec4*       addRgba  (const char* _name, const vec4&  _default, float _min, float _max, vec4*       storage_ = nullptr, const char* _displayName = nullptr);
	StringBase* addString(const char* _name, const char*  _default,                         StringBase* storage_ = nullptr, const char* _displayName = nullptr);
	StringBase* addPath  (const char* _name, const char*  _default,                         StringBase* storage_ = nullptr, const char* _displayName = nullptr);

	Property*   find(StringHash _nameHash);
	Property*   find(const char* _name) { return find(StringHash(_name)); }

	const char* getName() const { return (const char*)m_name; }

	bool        edit(bool _showHidden = false);
	friend bool Serialize(apt::SerializerJson& _serializer_, PropertyGroup& _prop_);

private:
	apt::String<32> m_name;
	eastl::vector_map<apt::StringHash, Property*> m_props;

};

////////////////////////////////////////////////////////////////////////////////
// Properties
////////////////////////////////////////////////////////////////////////////////
class Properties: private apt::non_copyable<Properties>
{
public:
	typedef apt::StringHash StringHash;

	Properties()  {}
	~Properties();

	PropertyGroup& addGroup(const char* _name);

	Property*      findProperty(const char* _name)   { return findProperty(StringHash(_name)); }
	Property*      findProperty(StringHash _nameHash);	

	PropertyGroup* findGroup(StringHash _nameHash);	
	PropertyGroup* findGroup(const char* _name) { return findGroup(StringHash(_name)); }

	bool           edit(bool _showHidden = false);
	friend bool    Serialize(apt::SerializerJson& _serializer_, Properties& _props_);

private:
	eastl::vector_map<apt::StringHash, PropertyGroup*> m_groups;

};


namespace refactor {

class Property;

///////////////////////////////////////////////////////////////////////////////
// Properties
///////////////////////////////////////////////////////////////////////////////
class Properties: private apt::non_copyable<Properties>
{
public:
	static constexpr const char* kDefaultGroupName = "_defaultGroup";

	typedef bool (EditFunc)(Property& _prop);  // return true if the value changed
	typedef void (DisplayFunc)(Property& _prop);
	typedef bool (SerializeFunc)(Serializer& _serializer_, Property& _prop); // return false if error

	static bool DefaultEditFunc(Property& _prop);
	static void DefaultDisplayFunc(Property& _prop);
	static bool DefaultSerializeFunc(Serializer& _serializer_, Property& _prop);

	static Properties* GetCurrent() { return s_current; }
	static Properties* GetDefault();

	// Add a new property to the current group. If _stroage is 0, memory is allocated internally. If the property already exists it is updated
	// with the new paramters, replacing storage if specified. 
	template <typename T>
	static T* Add(const char* _name, T _default, T _min, T _max, T* _storage = nullptr, const char* _displayName = nullptr)
	{
		return GetCurrent()->add<T>(_name, _default, _min, _max, _storage, _displayName);
	}
	template <typename T>
	static T* Add(const char* _name, T _default, T* _storage = nullptr, const char* _displayName = nullptr)
	{
		return GetCurrent()->add<T>(_name, _default, _storage, _displayName);
	}

	// Find an existing property. If _groupName is 0, search the current group first.
	static Property* Find(const char* _propName, const char* _groupName = nullptr)
	{
		return GetCurrent()->find(_propName, _groupName);
	}

	// Push/pop the current group. If _group doesn't exist, a new empty group is created.
	static void PushGroup(const char* _groupName)
	{
		GetCurrent()->pushGroup(_groupName);
	}
	static void PopGroup()
	{
		GetCurrent()->popGroup();
	}
	
	static Properties* Create();
	static void        Destroy(Properties*& _properties_);

private:
	typedef eastl::vector_map<StringHash, Property*>    PropertyMap;
	typedef eastl::vector_map<StringHash, PropertyMap*> GroupMap;
	typedef eastl::vector_map<StringHash, String<64> >  StringMap;
	typedef PropertyMap Group;
	
	static Properties* s_current;

	GroupMap  m_groups;
	StringMap m_groupNames;
	eastl::vector<Group*> m_groupStack;

	Properties();
	~Properties();
	
	template <typename T>
	T* add(const char* _name, T _default, T _min, T _max, T* _storage = nullptr, const char* _displayName = nullptr);
	template <typename T>
	T* add(const char* _name, T _default, T* _storage = nullptr, const char* _displayName = nullptr);

	Property* find(const char* _propName, const char* _groupName = nullptr);

	void pushGroup(const char* _groupName);
	void popGroup();

	Group*    newGroup(const char* _groupName);
	Property* findInGroup(StringHash _propName, const Group* _group) const;

};

///////////////////////////////////////////////////////////////////////////////
// Property
///////////////////////////////////////////////////////////////////////////////
class Property
{
	friend class Properties;
public:
	typedef Properties::EditFunc      EditFunc;
	typedef Properties::DisplayFunc   DisplayFunc;
	typedef Properties::SerializeFunc SerializeFunc;
	
	enum Type_
	{
	 // basic types
		Type_Bool,
		Type_Int,
		Type_Float,
		Type_String,

	 // user type (must provide custom edit/display/serialize funcs)
		Type_User,

		Type_Count
	};
	typedef int Type;

	const char*   getName() const                               { return (const char*)m_name;        }
	void          setName(const char* _name)                    { m_name = _name;                    }
	const char*   getDisplayName() const                        { return (const char*)m_displayName; }
	void          setDisplayName(const char* _displayName)      { m_displayName = _displayName;      }
	EditFunc*     getEditFunc() const                           { return m_editFunc;                 }
	void          setEditFunc(EditFunc* _editFunc)              { m_editFunc = _editFunc;            }
	DisplayFunc*  getDisplayFunc() const                        { return m_displayFunc;              }
	void          setDisplayFunc(DisplayFunc* _displayFunc)     { m_displayFunc = _displayFunc;      } 	

	Type          getType() const                               { return m_type;                     }
	int           getCount() const                              { return m_count;                    }

private:
	typedef apt::String<32> String;

	String          m_name          = nullptr;
	String          m_displayName   = nullptr;
	EditFunc*       m_editFunc      = Properties::DefaultEditFunc;
	DisplayFunc*    m_displayFunc   = Properties::DefaultDisplayFunc;
	SerializeFunc*  m_serializeFunc = Properties::DefaultSerializeFunc;
	Type            m_type          = Type_Count;
	int             m_count         = 0;
	bool            m_ownsData      = false;
	char*           m_data          = nullptr;
	char*           m_default       = nullptr;
	char*           m_min           = nullptr;
	char*           m_max           = nullptr;
};


} // namespace refactor


} // namespace frm
