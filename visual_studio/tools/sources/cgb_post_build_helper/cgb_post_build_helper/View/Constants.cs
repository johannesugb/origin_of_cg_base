using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Media;

namespace CgbPostBuildHelper.View
{
	static class Constants
	{
		public static readonly string DateTimeUiFormat = System.Threading.Thread.CurrentThread.CurrentUICulture.DateTimeFormat.FullDateTimePattern;

		public static readonly Brush ErrorBrushDark = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#ff1a66");
		public static readonly Brush ErrorBrushLight = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#ffccdd");

		public static readonly Brush WarningBrushDark = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#e64d00");
		public static readonly Brush WarningBrushLight = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#ffddcc");

		public static readonly Brush InfoBrushDark = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#0099cc");
		public static readonly Brush InfoBrushLight = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#ccf2ff");

		public static readonly Brush SuccessBrushDark = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#2d8659");
		public static readonly Brush SuccessBrushLight = (Brush)new System.Windows.Media.BrushConverter().ConvertFromInvariantString("#d9f2e6");
	}
}
