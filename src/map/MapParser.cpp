/*
 * MapParser.cpp
 *
 *  Created on: 3 oct. 2017
 *      Author: Valentin
 */

#include "map/MapParser.h"
#include "utils/Colors.h"
#include "tinyxml2.h"
#include "utils/Utils.h"
#include <iostream>
#include <string>
#include <stdio.h>
#include <algorithm>
#include "entities/EntityWall.h"
#include "math/Vector.h"

#define MAP_PATH "./Maps/"
#define TILE_OFFSET 0

std::map<std::string, WORD> MapParser::objectColorTypes;

Map* MapParser::loadMap(std::string name)
{
	std::cout << "Loading map...";


	// Prepare new map
	Map* map = nullptr;

	// Try to load conventional map
	if(loadMapData(name, &map, true))
	{
		return map;
	}

	std::cout << "FAIL" << std::endl;
	return nullptr;
}

bool MapParser::loadColorObjectTypes()
{
	std::cout << "Loading colors...";

	try
	{
		// Load and check XML state
		tinyxml2::XMLDocument objTypesXML;

		tinyxml2::XMLError a_error = objTypesXML.LoadFile((MAP_PATH + std::string("objecttypes.xml")).c_str());
		if(a_error != tinyxml2::XML_SUCCESS)
		{
			std::cout << "FAIL" << std::endl;
			return false;
		}

		tinyxml2::XMLNode* root = objTypesXML.FirstChild();
		//Skip first line
		tinyxml2::XMLElement* objectsRoot = (tinyxml2::XMLElement*) root->NextSibling();

		// Read all objects
		tinyxml2::XMLElement* objects = objectsRoot->FirstChildElement("objecttype");
		while (objects != nullptr)
		{

			// Read object properties
			WORD attribute = 0;
			const char* objectName = objects->Attribute("name");
			tinyxml2::XMLElement* properties = objects->FirstChildElement("property");

			while (properties != nullptr)
			{
				const char* propertyName = properties->Attribute("name");
				bool value = false;
				properties->QueryBoolAttribute("default", &value);

				if (value)
				{
					applyColorForProperty(propertyName, attribute);
				}

				// Next property
				properties = properties->NextSiblingElement("property");
			}

			// Add object type to object type list
			objectColorTypes[objectName] = attribute;

			// Next object
			objects = objects->NextSiblingElement("objecttype");
		}

		std::cout << "DONE" << std::endl;

		return true;
	}
	catch(...)
	{
		std::cout << "FAIL" << std::endl;
		return false;
	}
}


BufferRenderer* MapParser::loadMapData(std::string name, Map** outputMap, bool shallOutputMap)
{
	try
	{
		// Load and check XML state
		tinyxml2::XMLDocument mapXML;

		mapXML.LoadFile((MAP_PATH + name).c_str());

		tinyxml2::XMLNode* root = mapXML.FirstChild();
		//Skip first line
		tinyxml2::XMLElement* mapRoot = (tinyxml2::XMLElement*) root->NextSibling();

		//Load data layer
		tinyxml2::XMLElement* layer = mapRoot->FirstChildElement("layer");

		// Retrieves map size
		int mapWidth;
		int mapHeight;
		mapRoot->QueryIntAttribute("width", &mapWidth);
		mapRoot->QueryIntAttribute("height", &mapHeight);
		BufferRenderer* buffer = new BufferRenderer(mapWidth, mapHeight);
		Map* map = nullptr;

		if(shallOutputMap)
		{
			*outputMap = new Map(mapWidth, mapHeight, buffer);
			map = *outputMap;
		}


		//Decode map background

		tinyxml2::XMLElement* data = layer->FirstChildElement("data");

		std::vector<int> tiles;
		csvAsTileVector(data->GetText(), tiles);

		for (unsigned int i = 0; i < tiles.size(); i++)
		{
			int x = i % mapWidth;
			int y = i / mapWidth;

			int dataId = tiles[i];
			int tileId = dataId == 0 ? ' ' : dataId;

			buffer->getCharAt(x, y).Attributes = WHITE;
			buffer->getCharAt(x, y).Char.AsciiChar = tileId;
		}

		// Read objects layers
		tinyxml2::XMLElement* objectLayers = mapRoot->FirstChildElement("objectgroup");
		while (objectLayers != nullptr)
		{

			// Read objects inside layers
			tinyxml2::XMLElement* objects = objectLayers->FirstChildElement("object");
			while (objects != nullptr)
			{
				// Fetch object name
				const char* name = objects->Attribute("name");

				// Invisible objects (For debug)
				bool visible = true;
				objects->QueryBoolAttribute("visible", &visible);
				if (!visible)
				{
					objects = objects->NextSiblingElement("object");
					continue;
				}

				// Fetch object type
				const char* type = objects->Attribute("type");

				// Fetch object position
				int xPos = 0, yPos = 0, width = 0, height = 0;

				objects->QueryIntAttribute("x", &xPos);
				objects->QueryIntAttribute("y", &yPos);
				objects->QueryIntAttribute("width", &width);
				objects->QueryIntAttribute("height", &height);

				//Convert pixel position to tile position
				xPos /= 8;
				yPos /= 16;
				width /= 8;
				height /= 16;

				if(shallOutputMap)
				{
					if (strcmp(name, "Wall") == 0)
					{
						EntityWall* wall = new EntityWall(IVector2(xPos, yPos), new AABB(xPos, yPos, xPos + width, yPos + height, true));
						map->addEntity(wall);
					}
					else if (strcmp(name, "Platform") == 0)
					{
						EntityWall* wall = new EntityWall(IVector2(xPos, yPos), new AABB(xPos, yPos, xPos + width, yPos + height, true, true));
						map->addEntity(wall);
					}
					else if (strcmp(name, "Spawn") == 0)
					{
						IVector2 spawnPoint = IVector2(xPos, yPos);
						map->getSpawnPoints().push_back(spawnPoint);
					}
				}

				WORD objectAttribute;
				bool shallChangeAttr = false;
				std::vector<int> filters;

				// If object is color
				if (type && objectColorTypes.find(type) != objectColorTypes.end())
				{
					objectAttribute = objectColorTypes[type];
					shallChangeAttr = true;
				}

				//Object contains custom properties ?
				tinyxml2::XMLElement* customProperties = objects->FirstChildElement("properties");
				if (customProperties)
				{
					tinyxml2::XMLElement* customProperty = customProperties->FirstChildElement("property");

					while (customProperty != nullptr)
					{
						const char* propertyName = customProperty->Attribute("name");
						const char* propertyType = customProperty->Attribute("type");

						//Is string property
						if (!propertyType)
						{
							//Is filter
							if (strcmp(propertyName, "filter") == 0)
							{
								csvAsTileVector(customProperty->Attribute("value"), filters);
							}

						} //Is boolean property ?
						else if (strcmp(propertyType, "bool") == 0)
						{
							bool value = false;
							customProperty->QueryBoolAttribute("value", &value);
							applyColorForProperty(propertyName, objectAttribute, !value);
						}

						// Next property
						customProperty = customProperty->NextSiblingElement("property");
					}
				}

				// Apply color changes
				for (int x = xPos; x < xPos + width; x++)
				{
					for (int y = yPos; y < yPos + height; y++)
					{
						int tile = tiles[x + y * mapWidth] - 1;

						// Check filter is not set or if is set and contains correct tile
						if ((filters.size() == 0 || std::find(filters.begin(), filters.end(), tile) != filters.end()) && shallChangeAttr)
						{
							buffer->getCharAt(x, y).Attributes = objectAttribute;
						}
					}
				}

				// Next object
				objects = objects->NextSiblingElement("object");
			}

			// Next object layer
			objectLayers = objectLayers->NextSiblingElement("objectgroup");
		}

		return buffer;
	}
	catch(...)
	{
		return nullptr;
	}
}

bool MapParser::applyColorForProperty(const char* propertyName, WORD& attribute, bool removeProperty)
{
	if (strcmp(propertyName, "background-intense") == 0)
	{
		attribute = removeProperty ? attribute & ~BACKGROUND_INTENSITY : attribute | BACKGROUND_INTENSITY;
	}
	else if (strcmp(propertyName, "background-red") == 0)
	{
		attribute = removeProperty ? attribute & ~BACKGROUND_RED : attribute | BACKGROUND_RED;
	}
	else if (strcmp(propertyName, "background-green") == 0)
	{
		attribute = removeProperty ? attribute & ~BACKGROUND_GREEN : attribute | BACKGROUND_GREEN;
	}
	else if (strcmp(propertyName, "background-blue") == 0)
	{
		attribute = removeProperty ? attribute & ~BACKGROUND_BLUE : attribute | BACKGROUND_BLUE;
	}
	else if (strcmp(propertyName, "intense") == 0)
	{
		attribute = removeProperty ? attribute & ~FOREGROUND_INTENSITY : attribute | FOREGROUND_INTENSITY;
	}
	else if (strcmp(propertyName, "red") == 0)
	{
		attribute = removeProperty ? attribute & ~FOREGROUND_RED : attribute | FOREGROUND_RED;
	}
	else if (strcmp(propertyName, "green") == 0)
	{
		attribute = removeProperty ? attribute & ~FOREGROUND_GREEN : attribute | FOREGROUND_GREEN;
	}
	else if (strcmp(propertyName, "blue") == 0)
	{
		attribute = removeProperty ? attribute & ~FOREGROUND_BLUE : attribute | FOREGROUND_BLUE;
	}
	else
	{
		return false;
	}

	return true;
}

void MapParser::csvAsTileVector(std::string csvData, std::vector<int>& output)
{
	std::vector<string> tokens;
	split(tokens, csvData, ',');
	output.reserve(tokens.size());

	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		int dataId = atoi(tokens[i].c_str()) - TILE_OFFSET;
		output.push_back(dataId);
	}

}
