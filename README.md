# WixJsonExtension
#### An extension to Windows&reg; installer XML (WiX) to create or modify JSON-formatted files during an installation.

## Status: JSONPath supported for install only applications

[Windows&reg; installer XML](http://wixtoolset.org/) is an open-source set of tools to create Windows® software installation setups (\*.msi), using XML files to define the content and behavior of the setup.
One important step in most setups is to modify configuration files to reflect either settings specified during the setup by the user, or other settings specific to the individual installation.
Windows&reg; installer has built-in actions to modify the classic ini-files (\*.ini), and WiX provides extensions (and the required custom actions) to modify XML files, and in particular the Application Configuration files (\*.exe.config or \*.dll.config) used by Microsoft&reg; .NET applications.
Today, a third format has become popular, especially in web applications: [JSON](https://www.json.org/) - JavaScript Object Notation.
Often used in networking scenarios (REST services, JQuery, etc.), it is also used as a local data store format, including configuration files.
Especially [.NET Core](https://github.com/dotnet/core) relies heavily on JSON-formatted configuration files, both during development and runtime.

This extension for WiX provides methods to modify JSON files (\*.json) during a software installation.
The new XML elements provided by this extension, JsonFile and JsonConfig, work very similar to the existing extensions for XML files, [XmlFile](http://wixtoolset.org/documentation/manual/v3/xsd/util/xmlfile.html) and [XmlConfig](http://wixtoolset.org/documentation/manual/v3/xsd/util/xmlconfig.html)

## Acknowledgements
WixJsonExtension uses [jsoncons](https://github.com/danielaparker/jsoncons) by Daniel Parker to read and manipulate JSON files.
