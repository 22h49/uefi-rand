extern "C" {

	#include <efi.h>
	#include <efilib.h>

}

#include <cstdint>
#include <cstring>

EFI_STATUS cxx_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable);

extern "C" {

	EFI_STATUS
	EFIAPI
	efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	   InitializeLib(ImageHandle, SystemTable);
	   SystemTable->BootServices->SetWatchdogTimer(0, 0, 0, nullptr); // Disable UEFI's timer which after ~5min would reset the computer.
	   return cxx_main(ImageHandle, SystemTable);
	}

}

using namespace std;

uint16_t * screen = reinterpret_cast<uint16_t*>(0xb8000);

enum class Color {
    Black = 0,
    Blue = 1,
    Green = 2,
    Cyan = 3,
    Red = 4,
    Magenta = 5,
    Brown = 6,
    LightGray = 7,
    DarkGray = 8,
    LightBlue = 9,
    LightGreen = 10,
    LightCyan = 11,
    LightRed = 12,
    Pink = 13,
    Yellow = 14,
    White = 15,
};

EFI_SYSTEM_TABLE* systab;

void putstr(char16_t * ptr) {
    uefi_call_wrapper((void*)systab->ConOut->OutputString, 2, systab->ConOut, ptr);
}

void putint(size_t * ptr) {
    uefi_call_wrapper((void*)systab->ConOut->OutputString, 2, systab->ConOut, ptr);
}

void disableCursor() {
    uefi_call_wrapper((void*)systab->ConOut->EnableCursor, 2, systab->ConOut, 0);
}

void setcolor(Color foreground = Color::White, Color background = Color::Black, std::size_t blink = 0) {
    std::size_t attr = blink << 7 | ((std::size_t)background & 0x7) << 4 | (std::size_t)foreground;
    uefi_call_wrapper((void*)systab->ConOut->SetAttribute, 2, systab->ConOut, attr);
}

void setpos(std::size_t x, std::size_t y) {
    uefi_call_wrapper((void*)systab->ConOut->SetCursorPosition, 3, systab->ConOut, x, y);
}

void putcenter(char16_t * ptr, size_t l, size_t h, size_t ln) {
    setpos((80 - l)/2, (25-h)/2 + ln);
    putstr(ptr);
}

void sleep(std::size_t us) {
    uefi_call_wrapper((void*)BS->Stall, 1, us);
}

void setTextMode() {
    uefi_call_wrapper((void*)systab->ConOut->SetMode, 2, systab->ConOut, 0);
}


void clearScreen() {
    uefi_call_wrapper((void*)systab->ConOut->ClearScreen, 2, systab->ConOut);
}

EFI_STATUS readKeyStroke(EFI_INPUT_KEY * key){
    return uefi_call_wrapper((void*)systab->ConIn->ReadKeyStroke, 2, systab->ConIn, key);
}

std::size_t rdtsc() {
    std::size_t high = 0;
    std::size_t low = 0;
    __asm__ ("rdtsc" : "=a"(low), "=d"(high));
    return high << 32 | low;
}

void drawTitle() {
    setcolor(Color::Magenta);
    putcenter(u"██╗  ██╗ █████╗  ██████╗██╗  ██╗██████╗ ██████╗ \n\r", 48, 10, 0);
    putcenter(u"██║  ██║██╔══██╗██╔════╝██║ ██╔╝╚════██╗██╔══██╗\n\r", 48, 10, 1);
    putcenter(u"███████║███████║██║     █████╔╝  █████╔╝██████╔╝\n\r", 48, 10, 2);
    putcenter(u"██╔══██║██╔══██║██║     ██╔═██╗  ╚═══██╗██╔══██╗\n\r", 48, 10, 3);
    putcenter(u"██║  ██║██║  ██║╚██████╗██║  ██╗██████╔╝██║  ██║\n\r", 48, 10, 4);
    putcenter(u"╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝╚═════╝ ╚═╝  ╚═╝\n\r", 48, 10, 5);
    setcolor(Color::White);
    putcenter(u"by Mikolaj Gazeel", 17, 10, 8);
    setcolor(Color::LightGray);
    putcenter(u"Press ", 42, 10, 16);
    setcolor(Color::Red);
    putstr(u"q");
    setcolor(Color::LightGray);
    putstr(u" to exit, ");
    setcolor(Color::Green);
    putstr(u"s");
    setcolor(Color::LightGray);
    putstr(u" to start, or ");
    setcolor(Color::Brown);
    putstr(u"a");
    setcolor(Color::LightGray);
    putstr(u" for about");
    setcolor(Color::LightGray);
    putcenter(u"         (!c) 2020         ", 28, 10, 17);
}



void drawFrame(Color col, Color bg){
	setcolor(col, bg);
	char16_t* a;
	a = u"                                                                                ";

	for(size_t i = 0; i < 25; i++){
		for(size_t j = 0; j < 80; j++){
			a[j] = j == 0 ? 32 + (rdtsc() % 94) : 32 + ((rdtsc()*a[j-1]*17) % 94);
		}
		putstr(a);
	}
}

void drawAbout(){
	putcenter(u"┌────────────────────────────────────────────────┐ ", 51, 16, 0);
	putcenter(u"│                      About                     │░", 51, 16, 1);
	putcenter(u"│                                                │░", 51, 16, 2);
	putcenter(u"│          This is a simple EFI srand app        │░", 51, 16, 3);
	putcenter(u"│   Since this is a very poor source of entropy  │░", 51, 16, 4);
	putcenter(u"│  It is wise to use it in crypto implemenations │░", 51, 16, 5);
	putcenter(u"│            Written for the WWW16 quals         │░", 51, 16, 6);
	putcenter(u"│             Painstakingly compiled by          │░", 51, 16, 7);
	putcenter(u"│                  Mikolaj Gazeel                │░", 51, 16, 8);
	putcenter(u"│                                                │░", 51, 16, 9);
	putcenter(u"│        PROTIP: use c or v to change colors     │░", 51, 16, 10);
	putcenter(u"│          (no code form cmatrix was used)       │░", 51, 16, 11);
	putcenter(u"│                                                │░", 51, 16, 12);
	putcenter(u"│                      ", 51, 16, 13);
	setcolor(Color::Black, Color::LightGray);
	putstr(u"<Back>");
	setcolor(Color::LightGray);
	putstr(u"                    │░");
	putcenter(u"└────────────────────────────────────────────────┘░", 51, 16, 14);
	putcenter(u" ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░", 51, 16, 15);

}

EFI_STATUS cxx_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    systab = SystemTable;
    
    setTextMode();
    drawTitle();

    disableCursor();

    while(1){
	EFI_INPUT_KEY *keyu;
    	while(readKeyStroke(keyu) != EFI_SUCCESS) {}
    	char16_t v = (char16_t)(*keyu).UnicodeChar;

	if(v == 'q' || v == 'Q'){
		putcenter(u"Exiting in ", 14, 10, 17);
    		setcolor(Color::LightRed);
		putstr(u"5");
    		setcolor(Color::LightGray);
		putstr(u"..");
		sleep(1000000);
		putcenter(u"Exiting in ", 14, 10, 17);
    		setcolor(Color::LightRed);
		putstr(u"4");
    		setcolor(Color::LightGray);
		putstr(u"..");
		sleep(1000000);
		putcenter(u"Exiting in ", 14, 10, 17);
    		setcolor(Color::LightRed);
		putstr(u"3");
    		setcolor(Color::LightGray);
		putstr(u"..");
		sleep(1000000);
		putcenter(u"Exiting in ", 14, 10, 17);
    		setcolor(Color::LightRed);
		putstr(u"2");
    		setcolor(Color::LightGray);
		putstr(u"..");
		sleep(1000000);
		putcenter(u"Exiting in ", 14, 10, 17);
    		setcolor(Color::LightRed);
		putstr(u"1");
    		setcolor(Color::LightGray);
		putstr(u"..");
		sleep(1000000);
		return EFI_SUCCESS;
	}
	else if(v == 'a' || v == 'A'){
		clearScreen();
		drawAbout();
		
		while(1){
			EFI_INPUT_KEY *keyu;
    			while(readKeyStroke(keyu) != EFI_SUCCESS) {}
    			char16_t v = (char16_t)(*keyu).UnicodeChar;
			if(v == 0x0D){
				break;
			}
		}
		clearScreen();
		drawTitle();
	}
	else if(v == 's' || v == 'S'){
		clearScreen();
		Color col = Color::LightGreen;
		Color bg = Color::Black;
		while(1){
			drawFrame(col, bg);
			sleep(70000);

			EFI_INPUT_KEY *keyu;
    			if(readKeyStroke(keyu) != EFI_SUCCESS) {continue;}
    			char16_t v = (char16_t)(*keyu).UnicodeChar;
			if(v == 'c'){
				col = (Color)(((size_t)col + 1) % 16);
			} else if(v == 'v'){
				bg =  (Color)(((size_t)bg + 1) % 16);
			}else if(v == 'q'){
				break;
			}
			clearScreen();
		}
		clearScreen();
		setcolor(Color::White, Color::Black);
		clearScreen();
		setcolor(Color::LightGray, Color::Black);
		clearScreen();
		drawTitle();
	}
	else{
    		putcenter(&v, 25, 10, 17);
    		setpos((80 - 25)/2 + 1, (25-10)/2 + 17);
		putstr(u": command not recognized");
		sleep(1000000);
		putcenter(u"         (!c) 2020         ", 28, 10, 17);
	}
    }

    sleep(3000000000);

    return EFI_SUCCESS;
}
