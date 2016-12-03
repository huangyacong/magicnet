using System;
using System.Text;
using magicnetdll;
using System.Runtime.InteropServices;

namespace gate
{
    public class gate
    {
		public static void Main(String[] args)
		{
			start();
			Console.Read();
		}

        public static void start()
        {
            bool result = true;
            magicnet.GateInit("gate", 30 * 1000, 1000, 8888, 9999, 0);
            while (result) { magicnet.GateProcess(); }
            magicnet.GateFin();
        }
    }
}
