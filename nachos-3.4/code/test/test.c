void main()
{
        int file;
        char buf[500];

	Print("=== Test program initiated ===\n");
        file = Open("abc.txt");
        //void Write(char *buffer, int size, OpenFileId id);
        
        //int Read(char *buffer, int size, OpenFileId id);
        Read(buf, 500, file);
        Write("Hello World!\n", 13, file);
        Close(file);
        Print(buf);
	Print("Test program terminated!\n");
	Halt();	// Optional. Just print stats
}
