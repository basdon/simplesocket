Simple socket plugin to run on a SA-MP (San Andreas Multiplayer) server.
sa-mp.com

This is basically BlueG's Socket plugin from: https://github.com/pBlueG/Socket
I just made it simple (for me) and crash less (so far).
Simple as in: it only does UDP.
The original repo doesn't have any license so this one doesn't have either.

Note: receiving data is done without a thread and by putting sockets as
non-blocking, this should work but I've never worked with these before
so I don't know what I'm doing.

Use at your own risk. Obviously. No guarantee that this software will
not destroy your machine, server, dog or marriage.