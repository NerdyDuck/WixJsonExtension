using System.Reflection;
using System.Xml;
using System.Xml.Schema;
using Microsoft.Tools.WindowsInstallerXml;

namespace NerdyDuck.Wix.JsonExtension
{
    public sealed class JsonCompiler : CompilerExtension
    {
	    public JsonCompiler()
        {
            Schema = LoadXmlSchemaHelper(Assembly.GetExecutingAssembly(), "NerdyDuck.Wix.JsonExtension.Xsd.json.xsd");
        }

        public override XmlSchema Schema
        {
	        get;
        }

        /// <summary>
        /// Processes an element for the Compiler.
        /// </summary>
        /// <param name="sourceLineNumbers">Source line number for the parent element.</param>
        /// <param name="parentElement">Parent element of element to process.</param>
        /// <param name="element">Element to process.</param>
        /// <param name="contextValues">Extra information about the context in which this element is being parsed.</param>
        public override void ParseElement(SourceLineNumberCollection sourceLineNumbers, XmlElement parentElement, XmlElement element, params string[] contextValues)
        {
	        switch (parentElement.LocalName)
	        {
		        case "Component":
			        string componentId = contextValues[0];
			        string directoryId = contextValues[1];

			        switch (element.LocalName)
			        {
				        case "JsonFile":
					        ParseJsonConfigElement(element, componentId, directoryId);
					        break;
				        default:
					        Core.UnexpectedElement(parentElement, element);
					        break;
			        }
			        break;
		        default:
			        Core.UnexpectedElement(parentElement, element);
			        break;
	        }
        }

		/// <summary>
		/// Parses a RemoveFolderEx element.
		/// </summary>
		/// <param name="node">Element to parse.</param>
		/// <param name="componentId">Identifier of parent component.</param>
		/// <param name="parentDirectory">Identifier of parent component's directory.</param>
		private void ParseJsonConfigElement(XmlNode node, string componentId, string parentDirectory)
		{
			SourceLineNumberCollection sourceLineNumbers = Preprocessor.GetSourceLineNumbers(node);
			string id = null;
			string directory = null;
			int on = CompilerCore.IntegerNotSet;
			string PreserveModifiedDate = null;
			string dirProperty = parentDirectory; // assume the parent directory will be used as the "DirProperty" column.

			if (node.Attributes != null)
			{
				foreach (XmlAttribute attribute in node.Attributes)
				{
					if (0 == attribute.NamespaceURI.Length || attribute.NamespaceURI == Schema.TargetNamespace)
					{
						switch (attribute.LocalName)
						{
							case "Id": // Identifier for json file modification
								id = Core.GetAttributeIdentifierValue(sourceLineNumbers, attribute);
								break;
							case "File": // Path of the .json file to modify.
								directory = Core.GetAttributeIdentifierValue(sourceLineNumbers, attribute);
								Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "File", directory);
								break;
							case "ElementPath": // The path to the parent element of the element to be modified. The semantic can be either JSON Path or JSON Pointer language, as specified in the SelectionLanguage attribute. Note that this is a formatted field and therefore, square brackets in the path must be escaped. In addition, JSON Path and Pointer allow backslashes to be used to escape characters, so if you intend to include literal backslashes, you must escape them as well by doubling them in this attribute. The string is formatted by MSI first, and the result is consumed as the JSON Path or Pointer.
								break;
							case "Name": // Name of JSON property to modify. If the value is empty, then the parent element specified in ElementPath is the target of any changes.
								break;
							case "Value": // The value to set. May be one of the simple JSON types, or a JSON-formatted object. See the <html:a href="http://msdn.microsoft.com/library/aa368609(VS.85).aspx" target="_blank">Formatted topic</html:a> for information how to escape square brackets in the value.
								break;
							case "ValueType": // The type of Value.
								break;
							case "Action": // The type of modification to be made to the JSON file when the component is installed or uninstalled.
								break;
							case "On": // Defines when the specified changes to the JSON file are to be done.
								string onValue = Core.GetAttributeValue(sourceLineNumbers, attribute);
								if (onValue.Length == 0)
								{
									@on = CompilerCore.IllegalInteger;
								}
								else
								{
									switch (onValue)
									{
										case "install":
											@on = 1;
											break;
										case "uninstall":
											@on = 2;
											break;
										case "both":
											@on = 3;
											break;
										default:
											Core.OnMessage(WixErrors.IllegalAttributeValue(sourceLineNumbers, node.Name,
												"On", onValue, "install", "uninstall", "both"));
											@on = CompilerCore.IllegalInteger;
											break;
									}
								}

								break;
							case "PreserveModifiedDate": // Specifies whether or not the modification should preserve the modified date of the file.  Preserving the modified date will allow the file to be patched if no other modifications have been made.
								PreserveModifiedDate = Core.GetAttributeValue(sourceLineNumbers, attribute);
								break;
							case "Sequence": // Specifies the order in which the modification is to be attempted on the JSON file.  It is important to ensure that new elements are created before you attempt to modify them.
								break;
							case "SelectionLanguage": // Specify whether the JSON object should use JSON Path (default) or JSON Pointer as the query language for ElementPath.
								break;
							case "VerifyPath": // The path to the element being modified. This is required for 'delete' actions. For 'set' actions, VerifyPath is used to decide if the element already exists. The semantic can be either JSON Path or JSON Pointer language, as specified in the SelectionLanguage attribute. Note that this is a formatted field and therefore, square brackets in the path must be escaped. In addition, JSON Path and Pointer allow backslashes to be used to escape characters, so if you intend to include literal backslashes, you must escape them as well by doubling them in this attribute. The string is formatted by MSI first, and the result is consumed as the JSON Path or Pointer.
								break;
							default:
								Core.UnexpectedAttribute(sourceLineNumbers, attribute);
								break;
						}
					}
					else
					{
						Core.UnsupportedExtensionAttribute(sourceLineNumbers, attribute);
					}
				}
			}

			if (CompilerCore.IntegerNotSet == on)
			{
				Core.OnMessage(WixErrors.ExpectedAttribute(sourceLineNumbers, node.Name, "On"));
				on = CompilerCore.IllegalInteger;
			}

			if (null != directory && null != PreserveModifiedDate)
			{
				Core.OnMessage(WixErrors.IllegalAttributeWithOtherAttribute(sourceLineNumbers, node.Name, "Property", "Directory", directory));
			}
			else
			{
				if (null != directory)
				{
					dirProperty = directory;
				}
				else if (null != PreserveModifiedDate)
				{
					dirProperty = PreserveModifiedDate;
				}
			}

			id ??= string.Concat("Remove", dirProperty);

			foreach (XmlNode child in node.ChildNodes)
			{
				if (XmlNodeType.Element == child.NodeType)
				{
					if (child.NamespaceURI == Schema.TargetNamespace)
					{
						Core.UnexpectedElement(node, child);
					}
					else
					{
						Core.UnsupportedExtensionElement(node, child);
					}
				}
			}

			if (!Core.EncounteredError)
			{
				Row row = Core.CreateRow(sourceLineNumbers, "JsonConfig");
				row[0] = id;
				row[1] = componentId;
				row[2] = dirProperty;
				row[3] = on;

				Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "CustomAction", "JsonConfig");
			}
		}
	}
}
