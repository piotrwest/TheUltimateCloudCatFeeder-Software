To add a new library (example):

```
# In terminal:
cd RavenousCat_ArduinoProject/externalLibs
git submodule add https://github.com/bblanchon/ArduinoJson.git
cd ArduinoJson/
git checkout v5.13.3
# At the end, you may want to add the library to c_cpp_properties.json file in .vscode folder.
```