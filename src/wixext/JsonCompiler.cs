using System.Xml.Schema;
using Microsoft.Tools.WindowsInstallerXml;

namespace NerdyDuck.Wix.JsonExtension
{
    public sealed class JsonCompiler : CompilerExtension
    {
        private XmlSchema _schema;

        public JsonCompiler()
        {
            _schema = LoadXmlSchemaHelper(typeof(JsonCompiler).Assembly, "NerdyDuck.Wix.JsonExtension.Data.json.xsd");
        }
        public override XmlSchema Schema => _schema;
    }
}
