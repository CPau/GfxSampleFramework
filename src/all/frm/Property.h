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
// Simple property system. Use for application configs etc.
// - Instances of the Properties class are containers of named groups, which
//   are containers of named properties. Properties are therefore uniquely
//   identified by the name and group name.
// - Loading properties is order independent wrt the code which initializes
//   the properties, i.e. don't require the properties to be init before loading
//   or vice-versa. This is achieved by loading *everything* which is in the disk
//   file and then setting the value when the property is added from the code.
// 
// \todo
// - Need to allocate internal storage for all properties, the external storage
//   ptr *may* be invalidated. This means that code must explicitly invalidate
//   the external storage ptr when necessary (e.g. in the dtor).
// - Uint type?
///////////////////////////////////////////////////////////////////////////////
class Properties: private apt::non_copyable<Properties>
{
public:
	static constexpr const char* kDefaultGroupName = "_defaultGroup";

	typedef bool (EditFunc)(Property& _prop);  // return true if the value changed
	typedef void (DisplayFunc)(const Property& _prop);
		
	enum Type_
	{
		Type_Bool,
		Type_Int,
		Type_Float,
		Type_String,

		Type_Count
	};
	typedef int Type;
	static const char* GetTypeStr(Type _type);
	static int         GetTypeSizeBytes(Type _type);
	template <typename T>
	static Type        GetType();
	template <typename T>
	static int         GetCount();

	static bool DefaultEditFunc(Property& _prop_);
	static bool ColorEditFunc(Property& _prop_);
	static bool PathEditFunc(Property& _prop_);
	static void DefaultDisplayFunc(const Property& _prop);
	static void ColorDisplayFunc(const Property& _prop);

	static Properties* GetDefault();
	static Properties* GetCurrent()                        { return s_current; }
	static Properties* SetCurrent(Properties* _properties) { s_current = _properties ? _properties : GetDefault(); }

	// Add a new property to the current group. If _storage is 0, memory is allocated internally. If the property already exists it is updated with the new metadata. 
	template <typename T>
	static Property* Add(const char* _name, const T& _default, const T& _min, const T& _max, T* _storage = nullptr, const char* _displayName = nullptr)
	{
		return GetCurrent()->add<T>(_name, _default, _min, _max, _storage, _displayName);
	}
	template <typename T>
	static Property* Add(const char* _name, const T& _default, T* _storage = nullptr, const char* _displayName = nullptr)
	{
		return GetCurrent()->add<T>(_name, _default, _storage, _displayName);
	}

	static Property* AddColor(const char* _name, const vec3& _default, vec3* _storage = nullptr, const char* _displayName = nullptr);
	static Property* AddColor(const char* _name, const vec4& _default, vec4* _storage = nullptr, const char* _displayName = nullptr);
	static Property* AddPath(const char* _name, const PathStr& _default, PathStr* _storage = nullptr, const char* _displayName = nullptr);

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

	// Find a property as per Find(), invalidate the external storage ptr. Call this e.g. in the dtor of a class which owns the storage, this is
	// important to allow properties to be correctly serialized.
	static void InvalidateStorage(const char* _propName, const char* _groupName = nullptr);
	
	static Properties* Create();
	static void        Destroy(Properties*& _properties_);

	friend bool Serialize(apt::SerializerJson& _serializer_, Properties& _props_);

private:
	typedef eastl::vector_map<apt::StringHash, Property*>        PropertyMap;
	typedef eastl::vector_map<apt::StringHash, PropertyMap*>     GroupMap;
	typedef eastl::vector_map<apt::StringHash, apt::String<64> > StringMap;
	typedef PropertyMap Group;
	
	static Properties* s_current;

	GroupMap  m_groups;
	StringMap m_groupNames;
	eastl::vector<Group*> m_groupStack;

	Properties();
	~Properties();
	
	Property* add(const char* _name, Type _type, int _count, const void* _default, const void* _min, const void* _max, void* _storage, const char* _displayName);

	template <typename T>
	Property* add(const char* _name, const T& _default, const T& _min, const T& _max, T* _storage, const char* _displayName)
	{
		return add(_name, GetType<T>(), APT_TRAITS_COUNT(T), &_default, &_min, &_max, _storage, _displayName);
	}
	template <typename T>
	Property* add(const char* _name, const T& _default, T* _storage, const char* _displayName)
	{
		return add(_name, GetType<T>(), APT_TRAITS_COUNT(T), &_default, nullptr, nullptr, _storage, _displayName);
	}
		template <>
		Property* add<apt::StringBase>(const char* _name, const apt::StringBase& _default, apt::StringBase* _storage, const char* _displayName)
		{
			return add(_name, Type_String, 1, &_default, nullptr, nullptr, _storage, _displayName);
		}

	Property* find(const char* _propName, const char* _groupName = nullptr);

	void      pushGroup(const char* _groupName);
	void      popGroup();

	Group*    newGroup(const char* _groupName);
	Property* findInGroup(apt::StringHash _propName, const Group* _group) const;

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
	typedef Properties::Type          Type;

	void          reset();

	const char*   getName() const                               { return (const char*)m_name;        }
	void          setName(const char* _name)                    { m_name = _name;                    }
	const char*   getDisplayName() const                        { return (const char*)m_displayName; }
	void          setDisplayName(const char* _displayName)      { m_displayName = _displayName;      }
	EditFunc*     getEditFunc() const                           { return m_editFunc;                 }
	void          setEditFunc(EditFunc* _editFunc)              { m_editFunc = _editFunc;            }
	DisplayFunc*  getDisplayFunc() const                        { return m_displayFunc;              }
	void          setDisplayFunc(DisplayFunc* _displayFunc)     { m_displayFunc = _displayFunc;      }
	void*         getDefault()                                  { return m_default;                  }
	void          setDefault(void* _default);
	void*         getMin()                                      { return m_min;                      }
	void          setMin(void* _min);
	void*         getMax()                                      { return m_min;                      }
	void          setMax(void* _max);

	Type          getType() const                               { return m_type;                     }
	int           getCount() const                              { return m_count;                    }
	int           getSizeBytes() const;
	void*         getInternalStorage() const                    { return m_storageInternal;          }

	// Setting the external storage ptr to a none-null value will copy the value from internal -> external.
	// Setting the external storage ptr to null will copy external -> internal (invalidation).
	void*         getExternalStorage() const                    { return m_storageExternal;          }
	void          setExternalStorage(void* _storage_);

	// Return external storage ptr if not 0, else internal storage ptr.
	void*         getStorage() const                            { return m_storageExternal ? m_storageExternal : m_storageInternal; }

private:
	typedef apt::String<32> String;

	Property(
		const char* _name,
		const char* _displayName,
		Type        _type,
		int         _count,
		void*       _storageExternal,
		const void* _default,
		const void* _min,
		const void* _max
		);

	~Property();

	String          m_name            = nullptr;
	String          m_displayName     = nullptr;
	EditFunc*       m_editFunc        = Properties::DefaultEditFunc;
	DisplayFunc*    m_displayFunc     = Properties::DefaultDisplayFunc;
	Type            m_type            = Properties::Type_Count;
	int             m_count           = 0;
	char*           m_storageExternal = nullptr;
	char*           m_storageInternal = nullptr;
	char*           m_default         = nullptr;
	char*           m_min             = nullptr;
	char*           m_max             = nullptr;

	void copy(void* dst_, const void* _src);
};


} // namespace refactor


} // namespace frm
