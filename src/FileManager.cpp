// #include "pugixml.hpp"
// #include "FileManager.h"
// #include <iostream>

// std::vector<std::string> parseOSCFile(const std::string& filename)
// {
//     pugi::xml_document doc;
//     pugi::xml_parse_result result = doc.load_file(filename.c_str());
//     if (!result)
//     {
//         printf("OSC File Load Result: %s at offset (character position): %d\n", result.description(), result.offset);
//         return std::vector<std::string>();
//     }
//     std::vector<std::string> requiredFiles;
//     pugi::xml_node osc_root = doc.child("OpenSCENARIO");
//     if (!osc_root)
//     {
//         osc_root = doc.child("OpenScenario");
//     }

//     if (!osc_root)
//     {
//         printf("Couldn't find OpenSCENARIO or OpenScenario element - check XML!");
//     }

//     std::string oscPath = "";
//     std::string sgPath = "";
//     pugi::xml_node roadNetworkNode = osc_root.child("RoadNetwork");
//     for (pugi::xml_node roadNetworkChild = roadNetworkNode.first_child(); roadNetworkChild; roadNetworkChild = roadNetworkChild.next_sibling())
//     {
//         std::string roadNetworkChildName(roadNetworkChild.name());

//         if (roadNetworkChildName == "LogicFile")
//         {
//             oscPath = roadNetworkChild.attribute("filepath").value();
//         }
//         else if (roadNetworkChildName == "SceneGraphFile")
//         {
//             sgPath = roadNetworkChild.attribute("filepath").value();
//         }
//     }
//     //TODO 现在暂时只处理OpenDrive文件，不处理SceneGraphFile
//     if(!oscPath.empty())
//     {
//         requiredFiles.push_back(oscPath);
//     }

//     // std::vector<std::string> catalogDics;
//     // pugi::xml_node catalogsNode = osc_root.child("CatalogLocations");
//     // for (pugi::xml_node catalogsChild = catalogsNode.first_child(); catalogsChild; catalogsChild = catalogsChild.next_sibling())
//     // {
//     //     RegisterCatalogDirectory(catalogsChild);
//     // }

// }

// void onsuccess(void* arg, void* data, int dataSize) {
//     const char* filename = static_cast<const char*>(arg);
//     EM_ASM({
//         var filename = UTF8ToString($0);
//         var data = HEAPU8.subarray($1, $1 + $2);
//         FS.writeFile(filename, data);
//     }, filename, data, dataSize);
// }

// void onerror(void* arg, int errorCode, const char* errorText) {
//     // 处理错误
// }

