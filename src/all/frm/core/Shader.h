#pragma once

#include <frm/core/def.h>
#include <frm/core/gl.h>
#include <frm/core/math.h>
#include <frm/core/Resource.h>

#include <apt/String.h>
#include <EASTL/vector.h>
#include <EASTL/vector_map.h>

#include <initializer_list>

namespace frm {

////////////////////////////////////////////////////////////////////////////////
// ShaderDesc
// \note Virtual includes are processed during StageDesc::loadSource to make
//   management of line pragmas easier.
////////////////////////////////////////////////////////////////////////////////
class ShaderDesc
{
	friend class Shader;
public:
	// Set the default version string, e.g. "420 compatibility" (excluding the "#version" directive).
	static void        SetDefaultVersion(const char* _version);
	static const char* GetDefaultVersion() { return (const char*)s_defaultVersion; }

	ShaderDesc();

	// Set the version string, e.g. "420 compatibility" (excluding the "#version" directive).
	void        setVersion(const char* _version);
	const char* getVersion() const { return (const char*)m_version; }
	
	// Set source code path for _stage.
	void        setPath(GLenum _stage, const char* _path);
	const char* getPath(GLenum _stage) const;

	// Set source code for _stage (replace existing source code).
	void        setSource(GLenum _stage, const char* _src);
	const char* getSource(GLenum _stage) const;

	// Access source code dependencies.
	int         getDependencyCount(GLenum _stage) const;
	const char* getDependency(GLenum _stage, int _i) const;
	bool        hasDependency(const char* _path) const;

	// Set the local size (compute only).
	void         setLocalSize(int _x, int _y = 1, int _z = 1);
	const ivec3& getLocalSize() const { return m_localSize; }

	// Add a define to _stage. Supported types are int, uint, float, vec2, vec3, vec4, const char*.
	template <typename T>
	void addDefine(GLenum _stage, const char* _name, const T& _value);
	void addDefine(GLenum _stage, const char* _nameValue);
	// Add a define to all stages.
	template <typename T>
	void addGlobalDefine(const char* _name, const T& _value)
	{
		for (int i = 0; i < internal::kShaderStageCount; ++i) {
			addDefine(internal::kShaderStages[i], _name, _value);
		}
	}
	void addGlobalDefine(const char* _nameValue)
	{
		for (int i = 0; i < internal::kShaderStageCount; ++i) {
			addDefine(internal::kShaderStages[i], _nameValue);
		}
	}
	void        addGlobalDefines(std::initializer_list<const char*> _defines);
	void        clearDefines();
	void        clearDefines(GLenum _stage);
	int         getDefineCount(GLenum _stage) const;
	const char* getDefineName(GLenum _stage, int _i) const;
	const char* getDefineValue(GLenum _stage, int _i) const;


	// Add a virtual include. These replace instances of "#include _name" in the source code.
	// Virtual includes may not contain any additional #include directives (virtual or otherwise).
	void        addVirtualInclude(const char* _name, const char* _value);
	void        clearVirtualIncludes();	
	const char* findVirtualInclude(const char* _name) const;
	

	// Hash the version string, shader paths, defines, virtual includes and source (if present).
	uint64 getHash() const;

	// Return whether _stage is valid.
	bool hasStage(GLenum _stage) const;

private:
	typedef apt::String<sizeof("9999 compatibility\0")> VersionStr;
	typedef apt::String<64> Str;

	struct StageDesc
	{
		GLenum                      m_stage;
		Str                         m_path;         // Only if from a file.
		apt::String<0>              m_source;       // Excluding defines or virtual includes.
		eastl::vector_map<Str, Str> m_defines;      // Name, value.	
		eastl::vector<Str>          m_dependencies;

		bool isEnabled() const;
		bool hasDependency(const char* _path) const;
		bool loadSource(const ShaderDesc& _shaderDesc, const char* _path = nullptr);
		apt::String<0> getLogInfo() const;
	};
	
	static VersionStr           s_defaultVersion;
	VersionStr                  m_version;
	eastl::vector_map<Str, Str> m_vincludes; 
	StageDesc                   m_stages[internal::kShaderStageCount];
	ivec3                       m_localSize; // Compute only.

}; // class ShaderDesc

////////////////////////////////////////////////////////////////////////////////
// Shader
////////////////////////////////////////////////////////////////////////////////
class Shader: public Resource<Shader>
{
public:
	static Shader* Create(const ShaderDesc& _desc);

	// Load/compile/link directly from a set of paths. _defines is a list of strings e.g. { "DEFINE1 1", "DEFINE2 0" }.
	static Shader* CreateVsFs(const char* _vsPath, const char* _fsPath, std::initializer_list<const char*> _defines = {});
	static Shader* CreateVsGsFs(const char* _vsPath, const char* _gsPath, const char* _fsPath, std::initializer_list<const char*> _defines = {});
	static Shader* CreateCs(const char* _csPath, int _localX = 1, int _localY = 1, int _localZ = 1, std::initializer_list<const char*> _defines = {});
	static void    Destroy(Shader*& _inst_);

	// Reload any shaders dependent on _path.
	static void    FileModified(const char* _path);

	static void    ShowShaderViewer(bool* _open_);


	bool load() { return reload(); }

	// Attempt to reload/compile all stages and link the shader program.
	// Return false if any stage fails to compile, or if link failed. If a previous attempt to load the shader was successful the 
	// shader remains valid even if reload() fails.
	bool reload();
	
	// Retrieve the index of a program resource via glGetProgramResourceIndex.
	GLint getResourceIndex(GLenum _type, const char* _name) const;

	// Retrieve teh location of a named uniform via glGetUniformLocation;
	GLint getUniformLocation(const char* _name) const;
	

	GLuint getHandle() const { return m_handle; }
	
	const ShaderDesc& getDesc() const { return m_desc; }

	// Set the local size (compute only) and recompile.
	bool   setLocalSize(int _x, int _y, int _z);
	ivec3  getLocalSize() const { return m_desc.getLocalSize(); }

	// Given the width/height/depth of an output image, generate an appropriate dispatch size as ceil(texture size/group size).
	ivec3  getDispatchSize(int _outWidth, int _outHeight, int _outDepth = 1);
	ivec3  getDispatchSize(const Texture* _tx, int _level);

	// Add (or set) _defines on all stages and recompile. _defines is a list of strings e.g. { "DEFINE1 1", "DEFINE2 0" }.
	bool   addGlobalDefines(std::initializer_list<const char*> _defines);
	template <typename T>
	bool   addGlobalDefine(const char* _name, const T& _value)
	{
		for (int i = 0; i < internal::kShaderStageCount; ++i) {
			m_desc.addDefine(internal::kShaderStages[i], _name, _value);
		}
		return loadEnabledStagesAndLinkProgram(false);
	}

private:
	GLuint     m_handle;

	ShaderDesc m_desc;
	GLuint     m_stageHandles[internal::kShaderStageCount];

	// Get/free info log for a shader stage.
	static const char* GetStageInfoLog(GLuint _handle);
	static void FreeStageInfoLog(const char*& _log_);

	// Get/free info log for a shader program.
	static const char* GetProgramInfoLog(GLuint _handle);
	static void FreeProgramInfoLog(const char*& _log_);

	Shader(Id _id, const char* _name);
	~Shader();	

	// Load the specified stage, return status.
	bool loadStage(int _i, bool _loadSource = true);

	// Call loadStage for all enabled stages, call linkProgram() if no compilation errors.
	bool loadEnabledStagesAndLinkProgram(bool _loadSource = true);

	// Link stages to generate the final program.
	bool linkProgram();

	// Set the name automatically based on desc.
	void setAutoName();

}; // class Shader

} // namespace frm
