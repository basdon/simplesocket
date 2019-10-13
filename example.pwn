#include <a_samp.inc>
#include <simplesocket.inc>

main()
{
}

new ssocket:out, ssocket:in;
new buf[20];

public OnGameModeInit()
{
	ssocket_set_recv_wait(0); // default is 0 so this does nothing
	in = ssocket_create();
	out = ssocket_create();
	ssocket_listen(in, 7788);
	ssocket_connect(out, "127.0.0.1", 7788);
	// send a string, zero byte, and byte value 66
	strpack(buf, "Hello, world");
	buf{12} = 0;
	buf{13} = 66;
	ssocket_send(out, buf, 14);
}

public OnGameModeExit()
{
	ssocket_destroy(out);
	ssocket_destroy(in);
}

public SSocket_OnRecv(ssocket:handle, data[], len)
{
	if (handle == in) {
		// print the zero byte and byte value 66
		printf("zero byte: %d", data{12});
		printf("66 byte: %d", data{13});
		// get the sent string
		strunpack(buf, data);
		printf("string: %s", buf);

		// note: if the byte order is different from what strunpack
		//       expects, you can use ssocket_strunpack
		ssocket_strunpack(buf, data);
		printf("ssocket unpacked string: %s", buf);
	}
}

