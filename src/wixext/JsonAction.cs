using System;
using System.Collections.Generic;
using System.Text;

namespace NerdyDuck.Wix.JsonExtension
{
	internal enum JsonAction
	{
		DeleteValue = 1,
		SetValue = 2,
		AddArrayValue = 4,
	}
}
