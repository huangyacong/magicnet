using System;
using System.Text;
using magicnetdll;
using System.Runtime.InteropServices;

namespace game
{
    public class game
    {
        public static void Main(String[] args)
        {
           start();
           Console.Read();
        }

        public static void start()
        {
            byte[] pcBuf = new byte[1024 * 1024 * 4];
            magicnet.SvrInit("game", 30 * 1000, 9999);
            magicnet.RegSvr("game");

            while(true)
            {
                ulong rkRecvHSocket = 0;
                int riBufLen = 1024 * 1024 * 4;
                magicnet.MAGIC_STATE ret = magicnet.SvrRead(ref rkRecvHSocket, pcBuf, ref riBufLen);
                if (ret == magicnet.MAGIC_STATE.MAGIC_SHUTDOWN_SVR) { break; }
                if (ret == magicnet.MAGIC_STATE.MAGIC_IDLE_SVR_DATA) { magicnet.TimeSleep(1); }
                if (ret == magicnet.MAGIC_STATE.MAGIC_CLIENT_CONNECT) { Console.WriteLine("connect {0} {1} {2}", rkRecvHSocket, Encoding.ASCII.GetString(pcBuf, 0, riBufLen), riBufLen); }
                if (ret == magicnet.MAGIC_STATE.MAGIC_CLIENT_DISCONNECT) { Console.WriteLine("disconnect {0} ", rkRecvHSocket); }
                if (ret == magicnet.MAGIC_STATE.MAGIC_RECV_DATA_FROM_SVR) { Console.WriteLine("recv svr {0} {1} {2}", rkRecvHSocket, Encoding.ASCII.GetString(pcBuf, 0, riBufLen), riBufLen); }
                if (ret == magicnet.MAGIC_STATE.MAGIC_RECV_DATA_FROM_CLIENT) { magicnet.SvrSendClient(rkRecvHSocket, pcBuf, riBufLen); }
            }

            magicnet.SvrFin();
        }
    }
}
