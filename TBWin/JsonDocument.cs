using System;
using System.Collections.Generic;
using System.IO;
using System.Resources;
using System.Windows;
using Microsoft.Win32;

namespace TBWin
{
    public class JsonDocument
    {
        internal JsonDocument()
        {
        }

        internal JsonDocument(string path)
        {
            Open(path);
        }

        public string FileName
        {
            get
            {
                if (string.IsNullOrEmpty(_path))
                {
                    return _resourceManager.GetString("UntitledFilename");
                }
                else
                {
                    int startIndex = _path.LastIndexOf('\\') + 1;
                    return _path.Substring(startIndex);
                }
            }
        }

        public string DisplayName
        {
            get
            {
                using (RegistryKey filekey = Registry.CurrentUser.CreateSubKey(@"Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced"))
                {
                    if ((filekey != null) && (filekey.GetValue("HideFileExt", 0).ToString() == "0"))
                    {
                        return FileName + " - " + _resourceManager.GetString("ApplicationName");
                    }
                    else
                    {
                        int endIndex = _path.LastIndexOf('.');
                        return FileName.Substring(0, endIndex) + " - " + _resourceManager.GetString("ApplicationName");
                    }
                }
            }
        }

        private string InitialDirectory
        {
            get
            {
                if (string.IsNullOrEmpty(_path))
                {
                    return Environment.CurrentDirectory;
                }
                else
                {
                    return _path.Substring(0, _path.LastIndexOf('\\'));
                }
            }
        }

        public string Path
        {
            get { return _path; }
            set { _path = value; }
        }

        public IDictionary<string,dynamic> Content
        {
            get { return _content; }
            set { _content = value; }
        }

        public bool IsDirty
        {
            get { return _isDirty; }
            set { _isDirty = value; }
        }

        public bool Close()
        {
            bool result = true;
            if (_isDirty)
            {
                result = Save();
            }

            if (result)
            {
                _path = "";
                _content = null;
                _isDirty = false;
            }
            return result;
        }

        public bool SaveNew()
        {
            var dlg = new SaveFileDialog
            {
                Filter = "JSON Documents (*.json)|*.json|All Files|*.*",
                DefaultExt = "json"
            };
            if (dlg.ShowDialog() == true)
            {
                return Save(dlg.FileName);
            }
            return false;
        }

        public bool Save()
        {
            if (_path.Length == 0)
            {
                return SaveNew();
            }
            else
            {
                return Save(_path);
            }
        }

        public bool Save(string path)
        {
            try
            {
                string json = new TournamentConfigSerializer().Serialize(_content, true);
                using (var sw = new StreamWriter(path))
                {
                    _path = path;
                    sw.Write(json);
                    _isDirty = false;
                    return true;
                }
            }
            // This catch handles the exception raised when trying to save a read only file
            catch (UnauthorizedAccessException)
            {
                var appTitle = _resourceManager.GetString("ApplicationName");
                var message = _resourceManager.GetString("UnauthorizedAccessWarningMessage");
                if (message != null)
                {
                    message = string.Format(message, path);
                }
                MessageBox.Show(message, appTitle, MessageBoxButton.OK, MessageBoxImage.Exclamation);
                return SaveNew();
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
            }
            return false;
        }

        public bool Open()
        {
            var dlg = new OpenFileDialog
            {
                Filter = "JSON Documents (*.json)|*.json|All Files|*.*",
                DefaultExt = "json",
                InitialDirectory = InitialDirectory,
                Multiselect = false,
                CheckFileExists = true
            };
            if (dlg.ShowDialog() == true)
            {
                return Open(dlg.FileName);
            }
            return false;
        }

        public bool Open(string path)
        {
            if (!string.IsNullOrEmpty(path))
            {
                try
                {
                    using (var sr = new StreamReader(path))
                    {
                        var json = sr.ReadToEnd();
                        _path = path;
                        _content = new TournamentConfigSerializer().Deserialize(json);
                        _isDirty = false;
                    }
                    return true;
                }
                catch (ArgumentException)
                {
                    var appTitle = _resourceManager.GetString("ApplicationName");
                    var message = _resourceManager.GetString("PathSyntaxError");
                    MessageBox.Show(message, appTitle);
                }
                catch (DirectoryNotFoundException)
                {
                    var appTitle = _resourceManager.GetString("ApplicationName");
                    var message = _resourceManager.GetString("PathNotFoundError");
                    MessageBox.Show(message, appTitle);
                }
                catch (DriveNotFoundException)
                {
                    var appTitle = _resourceManager.GetString("ApplicationName");
                    var message = _resourceManager.GetString("PathNotFoundError");
                    MessageBox.Show(message, appTitle);
                }
                catch (FileNotFoundException)
                {
                    var appTitle = _resourceManager.GetString("ApplicationName");
                    var message = _resourceManager.GetString("FileNotFoundError");
                    if (message != null)
                    {
                        message = String.Format(message, path);
                    }
                    MessageBox.Show(message, appTitle);
                }
                catch (OutOfMemoryException)
                {
                    var appTitle = _resourceManager.GetString("ApplicationName");
                    var message = _resourceManager.GetString("OutOfMemoryError");
                    MessageBox.Show(message, appTitle);
                }
            }
            //if we haven't returned yet, then there was a problem. Proceed with blank doc
            _path = "";
            _content = null;
            _isDirty = false;
            return false;
        }

        private string _path = "";
        private IDictionary<string,dynamic> _content;
        private bool _isDirty;
        private readonly ResourceManager _resourceManager = Properties.Resources.ResourceManager;
    }
}