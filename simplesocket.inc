
// vim: set filetype=c ts=8 noexpandtab:

#define INVALID_SOCKET (-1)

/// <summary>
///   Connects given socket to given address, port.
///   On success you can send using ssocket_send.
/// </summary>
/// <returns>0 on failure.</returns>
native ssocket_connect(ssocket:handle, address[], port);

/// <summary>
///   Creates a new socket.
///   On success you can use ssocket_connect to prepare sending or
///   ssocket_listen to listen.
/// </summary>
/// <remarks>Max amount of sockets is 10 when I wrote this line.</remarks>
/// <returns>A socket handle or -1 on error.</returns>
native ssocket:ssocket_create();

/// <summary>Destroys a socket.</summary>
/// <returns>0 on failure.</returns>
native ssocket_destroy(ssocket:handle);

/// <summary>Set a socket to listen to a port.</summary>
/// <returns>0 on failure.</returns>
native ssocket_listen(ssocket:handle, port);

/// <summary>
///   Send data through a socket that was setup with ssocket_connect.
/// </summary>
/// <param name="data">Data to send, use packed strings for text.</param>
/// <param name="len">The length of the data in bytes.</param>
/// <returns>0 on failure.</returns>
native ssocket_send(ssocket:handle, data[], len);

/// <summary>
///   Set the amount of server ticks to wait between each data receive check.
///   Defaults to 0.
/// </summary>
/// <remarks>
///   Your server tickrate is defined by the sleep setting in server.cfg
///   When invoked, the current wait is reset and the next server tick will
///   check for received data.
///   Limited to 1000 (but why would you).
/// </remarks>
native ssocket_set_recv_wait(wait);

/// <summary>
///   Unpacks a string that is packed in a different byte order than what
///   the normal strunpack uses.
/// </summary>
/// <remarks>
///   Will write maximum maxlength bytes (always zero terminated)
/// </remarks>
native ssocket_strunpack(dest[], source[], maxlength=sizeof(dest));

/// <summary>
///   Callback called when a socket configured with ssocket_listen
///   receives data.
/// </summary>
/// <param name="data">Received data, is NOT zero terminated.</param>
/// <remarks>This is only called in the script that made the socket.</remarks>
forward SSocket_OnRecv(ssocket:handle, data[], len);
