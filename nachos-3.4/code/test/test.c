void main()
{
        int file = Open("abc.txt");
	Print("=== Test program initiated ===\n");
        //void Write(char *buffer, int size, OpenFileId id);
        //int Read(char *buffer, int size, OpenFileId id);
        Close(file);
	Print("--- Test program terminated ---\n");
	Halt();	// Optional. Just print stats
}
