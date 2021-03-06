/*
 * MapParser.h
 *
 *  Created on: 3 oct. 2017
 *      Author: Valentin
 */

#ifndef MAPPARSER_H_
#define MAPPARSER_H_

#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include "map/Map.h"

class MapParser
{
private:
	static std::map<std::string, WORD> objectColorTypes;

public:
	static Map* loadMap(std::string name);

	static BufferRenderer* loadMapData(std::string name, Map** outputMap, bool shallOutputMap = false);

	static bool loadColorObjectTypes();

private:
	static bool applyColorForProperty(const char* property, WORD& attribute, bool removeProperty = false);

	static void csvAsTileVector(std::string csvData, std::vector<int>& output);
};

#endif /* MAPPARSER_H_ */
