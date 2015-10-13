using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TBWin
{
    class TournamentSession
    {
        public static int ClientIdentifier()
        {
            if(Properties.Settings.Default.clientIdentifier == 0)
            {
                Random rand = new Random();
                int cid = rand.Next(10000, 100000);
                Properties.Settings.Default.clientIdentifier = cid;
            }

            return Properties.Settings.Default.clientIdentifier;
        }

        private static int _incrementingKey = 0;
        private static int CommandKey()
        {
            return _incrementingKey++;
        }


    }
}
