Stdio.File f;

int wx(mixed ... args)
{
//  write(">> ");
//  write(@args);
  return f->write(@args);
}
int main()
{
  f = Stdio.File("/dev/cu.usbmodem12341", "rw");
  wx("AT\r\n");
  werror("RES: %O\n", f->read(100, 1));

  wx("ATP\r\n");
  werror("RES: %O\n", f->read(100, 1));
  int x = 5<<(31-6);
  int w = 5<<29;
  for(int y = 0; y < 31; y++)
  { 
    write("%032b\r\n", x|w|(1<<32));
    wx("%04c\r\n", 0b1010101010101010101010101010101|(1<<31));
    wx("%04c\r\n", (0b1010101010101010101010101010101>>1)|(1<<31));
//    x<<=1;
 //   w>>=1;
    sleep(0.25);
return 1;
 }
/*
  write("%b%b%b%b\r\n", 0B11010001, 0B11011001, 0B11010001, 0B11010111);
 write("\n\n");
  wx("%c%c%c%c\r\n", 0B11010001, 0B11011001, 0B11010001, 0B11010111);
  write("%b%b%b%b\r\n", 0B11000001, 0B01000001, 0B00010000, 0B11000001);
  wx("%c%c%c%c\r\n", 0B11000001, 0B01000001, 0B00010000, 0B11000001);
*/
  wx("+++++");
  wx("ATS\r\n");
  string s;
  while(s = f->read(1, 1))
  { 
    write("<< " + s);
  }

  return 0;
}
