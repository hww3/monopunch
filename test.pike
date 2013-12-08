int main()
{
  Stdio.File f = Stdio.File("/dev/cu.usbmodem12341", "rw");

  f->write("AT\r\n");
  werror("RES: %O\n", f->read(100, 1));

  f->write("ATP\n");
  werror("RES: %O\n", f->read(100, 1));

  write("%b%b%b%b\r\n", 0B11010001, 0B11011001, 0B11010001, 0B11010111);
 write("\n\n");
  f->write("%c%c%c%c\r\n", 0B11010001, 0B11011001, 0B11010001, 0B11010111);
  f->write("%c%c%c%c\r\n", 0B11000001, 0B01000001, 0B00010000, 0B11000001);
  f->write("+++++");
  f->write("ATS\r\n");
  string s;
  while(s = f->read(1, 1))
  { 
    write(s);
  }

  return 0;
}
