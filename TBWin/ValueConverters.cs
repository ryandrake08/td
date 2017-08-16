using System;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Markup;
using Newtonsoft.Json.Linq;
using Xceed.Wpf.Toolkit.PropertyGrid.Converters;

namespace TBWin
{
    public abstract class BaseConverter : MarkupExtension
    {
        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }
    }

    public class UniversalValueConverter : BaseConverter, IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            // obtain the conveter for the target type
            var converter = TypeDescriptor.GetConverter(targetType);
            try
            {
                // determine if the supplied value is of a suitable type
                return converter.ConvertFrom(converter.CanConvertFrom(value.GetType()) ? value : value.ToString());
            }
            catch (Exception)
            {
                return value;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
    public class ValueExistsToBoolConverter : BaseConverter, IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value != null;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }

    public class MillisecondsToTimeSpanConverter : BaseConverter, IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var jvalue = value as JValue;
            if (jvalue != null)
            {
                return TimeSpan.FromMilliseconds(jvalue.ToObject<double>());
            }
            else
            {
                return null;
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var str = value as string;
            if (str != null)
            {
                return TimeSpan.Parse(str).TotalMilliseconds;
            }
            else
            {
                return null;
            }
        }
    }
}
