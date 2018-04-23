using System;
using System.Collections.Generic;
using System.Reflection;
using System.Text;
using Microsoft.Tools.WindowsInstallerXml;

namespace NerdyDuck.Wix.JsonExtension
{
    public sealed class WixJsonExtension : WixExtension
    {
        private JsonCompiler _compilerExtension;
        private JsonDecompiler _decompilerExtension;
        private TableDefinitionCollection _tableDefinitions;
        private Library _library;

        public override CompilerExtension CompilerExtension
        {
            get
            {
                if (_compilerExtension == null)
                {
                    _compilerExtension = new JsonCompiler();
                }
                return _compilerExtension;
            }
        }

        public override DecompilerExtension DecompilerExtension
        {
            get
            {
                if (_decompilerExtension == null)
                {
                    _decompilerExtension = new JsonDecompiler();
                }
                return _decompilerExtension;
            }
        }

        public override TableDefinitionCollection TableDefinitions
        {
            get
            {
                if (_tableDefinitions == null)
                {
                    _tableDefinitions = LoadTableDefinitionHelper(typeof(WixJsonExtension).Assembly, "NerdyDuck.Wix.JsonExtension.Data.tables.xml");
                }
                return _tableDefinitions;
            }
        }

        public override Library GetLibrary(TableDefinitionCollection tableDefinitions)
        {
            if (_library == null)
            {
                _library = LoadLibraryHelper(typeof(WixJsonExtension).Assembly, "NerdyDuck.Wix.JsonExtension.Data.json.wixlib", tableDefinitions);
            }
            return _library;
        }

        public override string DefaultCulture => "en-us";
    }
}
