import pcapy
import os
#Definer fil listen som vil indeholde alle Pcap filer
File_list = []
def main():
    
 #Vi finder alle Pcap filer i den folderen vi er i
 os.popen('ls | grep pcap > idunno.txt').read()
 Pcap_count = os.popen('wc -l idunno.txt').read()
 
 # Vi finder det reelle antal af filer af pcap typen
 File_list_lenght =''.join(c for c in "%s"%Pcap_count if c.isdigit())
 readfile= open('idunno.txt', 'r')
 if readfile:
     for lines in readfile:
         File_list.append(lines)
 readfile.close()
 Lenght = len(File_list)
 
 #Vi kan nu finde alt data sendt som set i pcap filen
 for i in range(0, len(File_list)):
 
 # Vi laeser fra vores oenskede pcap fil
  fil = File_list[i].rstrip()
  reader= pcapy.open_offline("%s"%fil)
 #Vi finder hvor mange frames der er i pcap filen
  Lenght = os.popen('tshark -r %s | wc -l' % fil).read()
  test = Lenght[0] + Lenght[1] + Lenght[2]
  PcapLenght = int(test)
 #print PcapLenght
  while True:
    try:
        Overheadsum = 0
        #Vi koerer igennem alle pakker/frames og finder deres laengder
        for x in range(0, PcapLenght):
         (header, payload) = reader.next();
         #print header.getlen();
         # Summer alle frames laengder
         Overheadsum = Overheadsum + header.getlen();
        
    except: pcapy.PcapError
    #print "FEJL"
    print Overheadsum
    break


main()
