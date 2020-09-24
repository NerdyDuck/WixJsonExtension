using Microsoft.Tools.WindowsInstallerXml;

namespace NerdyDuck.Wix.JsonExtension
{
    public sealed class WixJsonExtension : WixExtension
    {
        private JsonCompiler _compilerExtension;
        private JsonDecompiler _decompilerExtension;
        private TableDefinitionCollection _tableDefinitions;
        private Library _library;

	   public override CompilerExtension CompilerExtension => _compilerExtension ??= new JsonCompiler();

	   public override DecompilerExtension DecompilerExtension => _decompilerExtension ??= new JsonDecompiler();

	   public override TableDefinitionCollection TableDefinitions => _tableDefinitions ??= LoadTableDefinitionHelper(typeof(WixJsonExtension).Assembly,
				  "NerdyDuck.Wix.JsonExtension.Data.tables.xml");

	   public override Library GetLibrary(TableDefinitionCollection tableDefinitions) => _library ??= LoadLibraryHelper(typeof(WixJsonExtension).Assembly,
			   "NerdyDuck.Wix.JsonExtension.Data.json.wixlib", tableDefinitions);

	   public override string DefaultCulture => "en-us";
    }
}
