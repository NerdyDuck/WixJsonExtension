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
			string property = null;
			string dirProperty = parentDirectory; // assume the parent directory will be used as the "DirProperty" column.

			if (node.Attributes != null)
			{
				foreach (XmlAttribute attribute in node.Attributes)
				{
					if (0 == attribute.NamespaceURI.Length || attribute.NamespaceURI == Schema.TargetNamespace)
					{
						switch (attribute.LocalName)
						{
							case "Id":
								id = Core.GetAttributeIdentifierValue(sourceLineNumbers, attribute);
								break;
							case "File":
								directory = Core.GetAttributeIdentifierValue(sourceLineNumbers, attribute);
								Core.CreateWixSimpleReferenceRow(sourceLineNumbers, "File", directory);
								break;
							case "On":
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
							case "Property":
								property = Core.GetAttributeValue(sourceLineNumbers, attribute);
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

			if (null != directory && null != property)
			{
				Core.OnMessage(WixErrors.IllegalAttributeWithOtherAttribute(sourceLineNumbers, node.Name, "Property", "Directory", directory));
			}
			else
			{
				if (null != directory)
				{
					dirProperty = directory;
				}
				else if (null != property)
				{
					dirProperty = property;
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
