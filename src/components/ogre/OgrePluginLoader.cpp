/*
 * Copyright (C) 2014 Peter Szucs <peter.szucs.dev@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "OgrePluginLoader.h"

#include "services/config/ConfigService.h"

#include <OgreBuildSettings.h>
#include <OgreRoot.h>

#ifdef OGRE_STATIC_LIB

#ifdef OGRE_BUILD_PLUGIN_CG
#include <OgreCgPlugin.h>
#endif
#ifdef OGRE_BUILD_PLUGIN_OCTREE
#include <OgreOctreePlugin.h>
#endif
#ifdef OGRE_BUILD_PLUGIN_PFX
#include <OgreParticleFXPlugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GL
#include <OgreGLPlugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
#include <OgreGL3PlusPlugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES
#include <OgreGLESPlugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
#include <OgreGLES2Plugin.h>
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
#include <OgreD3D9Plugin.h>
#endif
//#ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
//#  include <OgreD3D11Plugin.h>
//#endif

#endif //ifdef OGRE_STATIC_LIB

namespace Ember
{
namespace OgreView
{
OgrePluginLoader::OgrePluginLoader()
{
#ifdef OGRE_STATIC_LIB
#ifdef OGRE_BUILD_RENDERSYSTEM_GL
	mPlugins.insert(PluginInstanceMap::value_type("RenderSystem_GL", OGRE_NEW Ogre::GLPlugin()));
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GL3PLUS
	mPlugins.insert(PluginInstanceMap::value_type("RenderSystem_GL3Plus", OGRE_NEW Ogre::GL3PlusPlugin()));
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES
	mPlugins.insert(PluginInstanceMap::value_type("RenderSystem_GLES", OGRE_NEW Ogre::GLESPlugin()));
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_GLES2
	mPlugins.insert(PluginInstanceMap::value_type("RenderSystem_GLES2", OGRE_NEW Ogre::GLES2Plugin()));
#endif
#ifdef OGRE_BUILD_RENDERSYSTEM_D3D9
	mPlugins.insert(PluginInstanceMap::value_type("RenderSystem_Direct3D9", OGRE_NEW Ogre::D3D9Plugin()));
#endif
	//#ifdef OGRE_BUILD_RENDERSYSTEM_D3D11
	//		mPlugins.insert(PluginInstanceMap::value_type("RenderSystem_Direct3D11", OGRE_NEW Ogre::D3D11Plugin()));
	//#endif
#ifdef OGRE_BUILD_PLUGIN_CG
	mPlugins.insert(PluginInstanceMap::value_type("Plugin_CgProgramManager", OGRE_NEW Ogre::CgPlugin()));
#endif
#ifdef OGRE_BUILD_PLUGIN_OCTREE
	mPlugins.insert(PluginInstanceMap::value_type("Plugin_OctreeSceneManager", OGRE_NEW Ogre::OctreePlugin()));
#endif
#ifdef OGRE_BUILD_PLUGIN_PFX
	mPlugins.insert(PluginInstanceMap::value_type("Plugin_ParticleFX", OGRE_NEW Ogre::ParticleFXPlugin()));
#endif
#else // ifndef OGRE_STATIC_LIB
	ConfigService& configSrv(EmberServices::getSingleton().getConfigService());
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	//on windows we'll bundle the dll files in the same directory as the executable
	mPluginDirs.push_back(".");
	mPluginExtension = ".dll";
#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX
	std::string pluginDir(configSrv.getPrefix());
	pluginDir += "/lib/OGRE";
	mPluginDirs.push_back(pluginDir);
	mPluginExtension = ".so";
#ifdef ENABLE_BINRELOC
	//binreloc might be used
	char* br_libdir = br_find_lib_dir(br_strcat(PREFIX, "/lib"));
	std::string libDir(br_libdir);
	free(br_libdir);
	mPluginDirs.push_back(libDir + "/OGRE");
#endif
	//enter the usual locations if Ogre is installed system wide, with local installations taking precedence
	mPluginDirs.push_back("/usr/local/lib/OGRE");
	mPluginDirs.push_back("/usr/lib/OGRE");
#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE
	// On Mac, plugins are found in Resources in the Main (Application) bundle, then in the Ogre framework bundle
	std::string pluginDir = configSrv.getSharedDataDirectory();
	mPluginDirs.push_back(pluginDir);
	pluginDir += "/../Plugins";
	mPluginDirs.push_back(pluginDir);
	mPluginExtension = ".dylib";
#endif
#ifdef OGRE_PLUGINDIR
	//also try with the plugindir defined for Ogre
	mPluginDirs.push_back(OGRE_PLUGINDIR);
#endif

#endif // ifndef OGRE_STATIC_LIB
}

OgrePluginLoader::~OgrePluginLoader()
{
}

void OgrePluginLoader::addPluginDir(const std::string& dir)
{
	mPluginDirs.insert(mPluginDirs.begin(), dir);
}

Ogre::Plugin* OgrePluginLoader::loadPlugin(const std::string& pluginName)
{
	auto it = mPlugins.find(pluginName);
	if (it != mPlugins.end()) {
		Ogre::Root::getSingleton().installPlugin(it->second);
		return it->second;
	}
#ifndef OGRE_STATIC_LIB
	// If the dynamic lib is not yet loaded, try to find and load it.
	// Load the shared library.
	Ogre::Plugin* plugin = loadDynPlugin(pluginName);
	if (plugin) {
		mPlugins.insert(PluginInstanceMap::value_type(pluginName, plugin));
		return plugin;
	}
#endif
	return nullptr;
}

Ogre::Plugin* OgrePluginLoader::getPlugin(const std::string& pluginName)
{
	auto it = mPlugins.find(pluginName);
	return (it != mPlugins.end()) ? it->second : nullptr;
}

bool OgrePluginLoader::unloadPlugin(const std::string& pluginName)
{
	auto it = mPlugins.find(pluginName);
	if (it != mPlugins.end()) {
		Ogre::Root::getSingleton().uninstallPlugin(it->second);
		return true;
	}
	return false;
}

void OgrePluginLoader::unloadPlugins()
{
	auto plugins = Ogre::Root::getSingleton().getInstalledPlugins();
	for (Ogre::Plugin* plugin : plugins) {
		plugin->uninstall();
	}
}

Ogre::Plugin* OgrePluginLoader::loadDynPlugin(const std::string& pluginName)
{
#ifndef OGRE_STATIC_LIB
	std::string pluginPath;
	bool pluginLoaded = false;
	for (const std::string& dir : mPluginDirs) {
		pluginPath = dir + "/" + pluginName + mPluginExtension;
		S_LOG_INFO("Trying to load the plugin '" << pluginPath << "'.");
		try {
			Ogre::Root::getSingleton().loadPlugin(pluginPath);
			pluginLoaded = true;
			break;
		} catch (...) {
			// Try debug version
			pluginPath = dir + "/" + pluginName + "_d" + mPluginExtension;
			S_LOG_INFO("Trying to load the plugin '" << pluginPath << "'.");
			try {
				Ogre::Root::getSingleton().loadPlugin(pluginPath);
				pluginLoaded = true;
				break;
			} catch (...) {
				// Failed to load plugin!
			}
		}
	}
	if (pluginLoaded) {
		S_LOG_INFO("Successfully loaded the plugin '" << pluginName << "' from '" << pluginPath << "'.");
	} else {
		S_LOG_FAILURE("Failed to load the plugin '" << pluginName << "'!");
	}
#else
	// Would work, but you should use static libs on static build to prevent strange bugs.
	assert(0);
#endif
	return nullptr;
}

}
}
