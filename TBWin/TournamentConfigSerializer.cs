using Newtonsoft.Json;
using System.Collections.Generic;

namespace TBWin
{
    public class TournamentConfigSerializer
    {
        public IDictionary<string,dynamic> Deserialize(string input)
        {
            return JsonConvert.DeserializeObject<IDictionary<string, dynamic>>(input);
        }

        public string Serialize(dynamic obj, bool indent=false)
        {
            return JsonConvert.SerializeObject(obj, indent ? Formatting.Indented : Formatting.None);
        }
    }
}
