About
=====

This plugins intended to disable automatic workspace building in Eclipse when computer running on battery.

Overview
========

Now there are two plugins: main plugin & fragment with native module for 32-bit OS X (I'm using 32-bit Eclipse). It would be nice if someone will write native modules for other platforms.

I know that such feature may be implemented simpler (with polling some console command, which output will show if we're on battery or not), without natives, but I wanted to learn some JNI: how to call native code from java and vice versa, so here it is.

Status
======

Now it just works, but some places can be improved.