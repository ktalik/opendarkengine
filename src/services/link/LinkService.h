/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/


#ifndef __LINKSERVICE_H
#define __LINKSERVICE_H

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "DatabaseService.h"
#include "FileGroup.h"
#include "LinkCommon.h"
#include "Relation.h"
#include "SharedPtr.h"

namespace Opde {

	/** @brief Link service - service managing in-game object links
	*/
	class LinkService : public Service {
		public:
			LinkService(ServiceManager *manager, const std::string& name);

			virtual ~LinkService();

			/** Sets the Relation chunk version */
			void setChunkVersion(uint major, uint minor) {
				mRelVMaj = major;
				mRelVMin = minor;
			}

			/** Convert the relation name to a flavor 
			  * @param name the relation name
			  * @return int The relation id (Flavor), or zero if not found */
			int nameToFlavor(const std::string& name);
			
			/** Convert the relation name to a flavor 
			  * @param name the relation name
			  * @return int The relation id (Flavor), or empty string if not found */
			std::string flavorToName(int flavor);

			/** Creates a relation type (link kind)
			* @param id The ID the relation will have (>0)
			* @param name The relation name
			* @param type The type defining the data format for link data
			* @param hidden The hidden relations (true) will not show up on public link list places */
			RelationPtr createRelation(const std::string& name, DTypeDefPtr type, bool hidden);

			/** Get relation given it's name
			* @param name The relation's name
			* @return A shared_pointer to the relation, or NULL if not found
			* @note The relation will be .isNull() if it was not found
			*/
			RelationPtr getRelation(const std::string& name);
			
			/** Get relation given it's flavor
			* @param flavor The relation's flavor
			* @return A shared_pointer to the relation, or NULL if not found
			* @note The relation will be .isNull() if it was not found
			*/
			RelationPtr getRelation(int flavor);

		protected:
            bool init();
            void bootstrapFinished();

			/** Database change callback */
            void onDBChange(const DatabaseChangeMsg& m);

			/** load links from a single database */
			void _load(FileGroupPtr db);

			/** Saves the links and link data according to the saveMask */
			void _save(FileGroupPtr db, uint saveMask);



			/** Clears all the data and the relation mappings */
			void _clear();

			/** request a mapping Name->Flavor and reverse
			* @param id The flavor value requested
			* @param name The name for that flavor (Relation name)
			* @param rel The relation instance to associate with that id
			* @return false if conflict happened, true if all went ok, and new mapping is inserted (or already was registered)
			*/
			bool requestRelationFlavorMap(int id, const std::string& name, RelationPtr rel);

			typedef std::map<int, std::string> FlavorToName;
			typedef std::map<std::string, int> NameToFlavor;

			/// Name to Relation instance. The primary storage of Relation instances.
			typedef std::map<std::string, RelationPtr> RelationNameMap;

			/// ID to Relation instance. Secondary storage of Relation instances, mapped per request when loading
			typedef std::map<int, RelationPtr> RelationIDMap;

			FlavorToName mFlavorToName;
			NameToFlavor mNameToFlavor;
			RelationIDMap mRelationIDMap;
			RelationNameMap mRelationNameMap;

			/// Relations chunk versions
			uint mRelVMaj, mRelVMin;

            /// Database callback
            DatabaseService::ListenerPtr mDbCallback;

            /// Database service
            DatabaseServicePtr mDatabaseService;
	};

	/// Shared pointer to Link service
	typedef shared_ptr<LinkService> LinkServicePtr;

	/// Factory for the LinkService objects
	class LinkServiceFactory : public ServiceFactory {
		public:
			LinkServiceFactory();
			~LinkServiceFactory() {};

			/** Creates a LinkService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();

		private:
			static std::string mName;
	};
}


#endif
