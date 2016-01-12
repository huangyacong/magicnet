using System;
using System.Text;
using System.Runtime.InteropServices;

namespace magicnetdll
{
    internal class magicnet
    {
        public enum MAGIC_STATE { MAGIC_SHUTDOWN_SVR = -1, MAGIC_IDLE_SVR_DATA, MAGIC_CLIENT_CONNECT, MAGIC_CLIENT_DISCONNECT, MAGIC_RECV_DATA_FROM_SVR, MAGIC_RECV_DATA_FROM_CLIENT };

        [DllImport("magicnet.dll", EntryPoint = "GateInit", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern bool GateInit(string strLogName, int iTimeOut, ushort usMax, ushort usOutPort, ushort usInPort);

        [DllImport("magicnet.dll", EntryPoint = "GateFin", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern void GateFin();

        [DllImport("magicnet.dll", EntryPoint = "GateProcess", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern void GateProcess();


        [DllImport("magicnet.dll", EntryPoint = "SvrInit", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern bool SvrInit(string strLogName, int iTimeOut, ushort usInPort);

        [DllImport("magicnet.dll", EntryPoint = "SvrFin", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern void SvrFin();

        [DllImport("magicnet.dll", EntryPoint = "RegSvr", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern bool RegSvr(string strSvrName);

        [DllImport("magicnet.dll", EntryPoint = "SvrSendClient", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern bool SvrSendClient(ulong kHSocket, [MarshalAs(UnmanagedType.LPArray)] byte[] buf, int iLen);

        [DllImport("magicnet.dll", EntryPoint = "SvrBindClient", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern void SvrBindClient(ulong kHSocket, string strSvrName);

        [DllImport("magicnet.dll", EntryPoint = "SvrCloseClient", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern void SvrCloseClient(ulong kHSocket);

        [DllImport("magicnet.dll", EntryPoint = "SvrSendSvr", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern bool SvrSendSvr(string strSvrName, [MarshalAs(UnmanagedType.LPArray)] byte[] buf, int iLen);

        [DllImport("magicnet.dll", EntryPoint = "SvrRead", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern MAGIC_STATE SvrRead(ref ulong rkRecvHSocket, [MarshalAs(UnmanagedType.LPArray)] byte[] buf, ref int riBufLen);


        [DllImport("magicnet.dll", EntryPoint = "TimeSleep", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern void TimeSleep(int iMiSec);

        [DllImport("magicnet.dll", EntryPoint = "TickCount", CharSet = CharSet.Ansi, CallingConvention = CallingConvention.StdCall, ExactSpelling = true)]
        public static extern ulong TickCount();
    }
}
