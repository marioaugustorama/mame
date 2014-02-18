/****************************************************************************************

  Jubilee Double-Up Poker
  -----------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Double-Up Poker,  198?,  Jubilee.


*****************************************************************************************

  Hardware Notes:
  ---------------

  PCB etched:

  HERBER LTD
  Jubilee Sales Pty Ltd.
  BD No V63-261 ISS1.

  1x TMS9980 CPU
  1x MC6845P CRTC
  
  1x TC5517AP-2 (2048 words x 8 bits Asynchronous CMOS Static RAM)
  1x 2114       (1024 words x 4 bits RAM)

  3x 2732 labelled 1, 2 and 3.
  3x 2764 labelled Red Blue and Green.

  1x 6.0 MHz crystal.


  From some forums...
  
  The memory chips on board are the toshiba TC5517ap powered via the Lithium cell and
  the 2114 find the 74148 or 74ls148, all 9980 cpu's use one, pin3 will generate a reset.
  The 5517 ram is compatible with 6116 or 2018.

  The 9980 has 3 interupt inputs, but they are binary.
  The ls148 encodes the interupts to the cpu - the highest interupt is reset.

  I tried pulling pin 3 of the 74ls148 low and yes, this sets up the reset interupt
  on pins 23, 24 and 25. The TC5517 checks out ok as a 6116.

  The crystal seems ok and this clock makes it throught to pin 34 of the MPU.
  I was expecting that PHASE 03 - pin 22, should be clocking, but it simply sits at 5v.

*****************************************************************************************

  *** Game Notes ***

  Nothing, yet...


*****************************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0000-2FFF    ; ROM space.
  3000-33FF    ; Video RAM.
  3400-37FF    ; Working RAM.
  3800-3BFF    ; Color (ATTR) RAM.
  3E00-3E03    ; CRTC Controller.

  CRU...

  0080-0080    ; ??? Read.
  00C8-00C8    ; Multiplexed Input Port
  0CC0-0CC6    ; Input Port mux selector?


  TMS9980A memory map:

  0000-0003 ---> Reset
  0004-0007 ---> Level 1
  0008-000B ---> Level 2
  000C-000F ---> Level 3
  0010-0013 ---> Level 4

  3FFC-3FFF ---> Load


*****************************************************************************************

  DRIVER UPDATES:


  [2014-02-17]

  - Corrected the crystal value and derivate clocks via #DEFINE.
  - Improved memory map.
  - Hooked the CRT controller, but the init sequence seems incomplete.
  - Created the accurate graphics banks.
  - Found and mapped the video RAM.
  - Hooked the ATTR RAM.
  - Assigned the correct graphics banks to the proper drawn tiles.
  - Find and mapped an input port.
  - Started a preliminary workaround to demux the input port.
  - Added technical notes.

  [2010-09-05]

  - Initial release.
  - Decoded graphics.
  - Proper tile colors.
  - Hooked the correct CPU.
  - Preliminary memory map.
  - Added technical notes.


  TODO:

  - Improve the CRU map.
  - Where is Coin In? Interrupts issue?
  - Confirm the CRT controller offset.
  - Sound.
  - Check clocks on a PCB (if someday appear!)


****************************************************************************************/

#define MASTER_CLOCK    XTAL_6MHz              /* confirmed */
#define CPU_CLOCK      (MASTER_CLOCK / 2)      /* guess */
#define CRTC_CLOCK     (MASTER_CLOCK / 4)      /* guess */

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "video/mc6845.h"
#include "machine/nvram.h"


class jubilee_state : public driver_device
{
public:
	jubilee_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	UINT8 mux_sel;
	
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(jubileep_videoram_w);
	DECLARE_WRITE8_MEMBER(jubileep_colorram_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_WRITE8_MEMBER(unk_w);
	DECLARE_READ8_MEMBER(mux_port_r);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_jubileep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(jubileep_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*************************
*     Video Hardware     *
*************************/

WRITE8_MEMBER(jubilee_state::jubileep_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(jubilee_state::jubileep_colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(jubilee_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    ---- --xx   bank select.
    xxxx xx--   seems unused.
*/
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int bank = (attr & 0x03);
	int color = 0;						/* fixed colors: one rom for each R, G and B. */

	SET_TILE_INFO_MEMBER(m_gfxdecode, bank, code, color, 0);
}


void jubilee_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(tilemap_get_info_delegate(FUNC(jubilee_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 jubilee_state::screen_update_jubileep(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void jubilee_state::palette_init()
{
}


/**************************
*  Read / Write Handlers  *
**************************/

INTERRUPT_GEN_MEMBER(jubilee_state::jubileep_interrupt)
{
	m_maincpu->set_input_line(INT_9980A_LEVEL1, ASSERT_LINE);
}


/*************************
* Memory Map Information *
*************************/
//59a
static ADDRESS_MAP_START( jubileep_map, AS_PROGRAM, 8, jubilee_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x2fff) AM_ROM
	AM_RANGE(0x3000, 0x33ff) AM_RAM AM_WRITE(jubileep_videoram_w) AM_SHARE("videoram")		/* First half of TC5517AP RAM */
	AM_RANGE(0x3400, 0x37ff) AM_RAM AM_SHARE("nvram")										/* Second half of TC5517AP RAM */
	AM_RANGE(0x3800, 0x3bff) AM_RAM AM_WRITE(jubileep_colorram_w) AM_SHARE("colorram")      /* Whole 2114 RAM (attr for gfx banks 00-03) */

/*	CRTC seems to be mapped here. Read 00-01 and then write on them.
    Then does the same for 02-03. Initialization is incomplete since
    set till register 0x0D.
*/
	AM_RANGE(0x3e00, 0x3e01) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)	// Incomplete... Till reg 0x0D
	AM_RANGE(0x3e02, 0x3e03) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)	// Incomplete... Till reg 0x0D

/* CRTC address: $3E01; register: $3E03
   CRTC registers: 2f 20 25 64 26 00 20 23 00 07 00 00 00
   screen total: (0x2f+1)*8 (0x26+1)*8  ---> 384 x 312
   visible area: 0x20 0x20  ---------------> 256 x 256
*/
ADDRESS_MAP_END

/*
  Video RAM =   3000-33FF
  Working RAM = 3400-37ff
  Color RAM =   3800-3bff (lower 4-bits)
*/

/*
    TODO: I/O lines handling. This is still work to be done; someone needs to
    check the schematics. Here, we need to deliver some reasonable return values
    instead of the 0. Returning a random number will create a nondeterministic
    behavior at best.
*/
READ8_MEMBER(jubilee_state::unk_r)
/* bits... -x-- --xx
            |     ||
            |     ++-- Credits.
            +--------- Bypass memory start error.

*/
{
	return (machine().rand() & 0x43);	// bit0 and bit1 are involved in credits input.
	logerror("CRU read from address %04x\n", offset<<4);
//	return 0;
}

WRITE8_MEMBER(jubilee_state::unk_w)
{
//  return (machine().rand() & 0xff);
	logerror("CRU write to address %04x: %d\n", offset<<1, data & 1);

	// In particular, the interrupt from above must be cleared. We assume that
	// this is done by one of the output lines, and from the 32 lines that are
	// set right after an interrupt is serviced, all are set to 0, and only one
	// is set to one. Maybe this one clears the interrupt.
	// TODO: Check the schematics.
	if (((offset<<1)==0x0ce2)&&(data==1))
	{
		m_maincpu->set_input_line(INT_9980A_LEVEL1, CLEAR_LINE);
	}

	// Inputs Multiplexion...

	if (((offset<<1)==0x0cc2)&&(data==1))
	{
		mux_sel = 1;
	}

	if (((offset<<1)==0x0cc4)&&(data==1))
	{
		mux_sel = 2;
	}

	if (((offset<<1)==0x0cc6)&&(data==1))
	{
		mux_sel = 3;
	}


	// suspicious... just for testing not ce0 ce2 ce6
//	if (((offset<<1)==0x0cc0)&&(data==0))
//	{
//		mux_sel = 6;
//	}
//	if (((offset<<1)==0x0ce2)&&(data==1))
//	{
//		mux_sel = 6;
//	}
//	if (((offset<<1)==0x0ce6)&&(data==1))
//	{
//		mux_sel = 6;
//	}
}

READ8_MEMBER(jubilee_state::mux_port_r)
{
	switch( mux_sel )
	{
		case 0x01: return ioport("IN0")->read();
		case 0x02: return ioport("IN1")->read();
		case 0x03: return ioport("IN2")->read();
	}

	return 0xff;
//	return (machine().rand() & 0xff);
}


static ADDRESS_MAP_START( jubileep_cru_map, AS_IO, 8, jubilee_state )
//	AM_RANGE(0x0000, 0x01ff) AM_READ(unk_r)
//	AM_RANGE(0x0080, 0x0080) AM_READ(unk_r)
//	AM_RANGE(0x00c8, 0x00c8) AM_READ(unk_r)		    // use to see the game stuff (even cards)
	AM_RANGE(0x00c8, 0x00c8) AM_READ(mux_port_r)	// Multiplexed inputs
	AM_RANGE(0x0000, 0x0fff) AM_WRITE(unk_w)
ADDRESS_MAP_END

/* I/O byte R/W

   0x080  R    ; Input port? polled at begining.
   0x0C8  R    ; Input port. If you tie it to a rnd value, you can see the game running.

   Can't see more inputs. There is a multiplexion with the following offsets as selectors:
   CC2/CC4/CC6
   
   None of them seems involved in the coin input.

*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( jubileep )
/* Coin In is tied to bits 0 & 1 (together) of port 0xC8.
   Need to find the selector status to implement in some way.
   (if you inject random data masked in port 0xC8, you can see 
   credits entering in the game)
*/
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )   PORT_NAME("Cancel / Take")			// cancel / take
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_BET )     PORT_NAME("Bet / Gamble")			// bet / gamble
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 / Half Gamble")	// hold 4 / half gamble
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD5 )    PORT_NAME("Hold 5 / Red")			// hold 5 / red
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE )        PORT_CODE(KEYCODE_8) PORT_NAME("Attendant Hand Pay")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 / Black")		// hold 1 / black
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2")				// hold 2
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3")				// hold 3

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) 	PORT_CODE(KEYCODE_9) PORT_NAME("Attendant (to pass the memory error and hand pay)")		// attendant (to pass the memory error and hand pay)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE )   PORT_CODE(KEYCODE_0) PORT_NAME("Bookkeeping")  PORT_TOGGLE		// service / bookeeping

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE )	PORT_CODE(KEYCODE_R) PORT_NAME("Reset")		// reset?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )		PORT_NAME("Deal/Start")		// deal
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tilelayout =
{
	8, 8,
	0x100,
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( jubileep )		/* 4 different graphics banks */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x0800, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0x1800, tilelayout, 0, 16 )
GFXDECODE_END


/***********************
*    CRTC Interface    *
************************/

static MC6845_INTERFACE( mc6845_intf )
{
	false,      /* show border area */
	0,0,0,0,    /* visarea adjustment */
	8,          /* number of pixels per video memory address */
	NULL,       /* before pixel update callback */
	NULL,       /* row update callback */
	NULL,       /* after pixel update callback */
	DEVCB_NULL, /* callback for display state changes */
	DEVCB_NULL, /* callback for cursor state changes */
	DEVCB_NULL, /* HSYNC callback */
	DEVCB_NULL, /* VSYNC callback */
	NULL        /* update address callback */
};

static TMS9980A_CONFIG( cpuconf )
{
	DEVCB_NULL,
	DEVCB_NULL,     // Instruction acquisition
	DEVCB_NULL,     // Clock out
	DEVCB_NULL,     // Hold acknowledge
	DEVCB_NULL      // DBIN
};

/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( jubileep, jubilee_state )

	/* basic machine hardware */
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, CPU_CLOCK, jubileep_map, jubileep_cru_map, cpuconf)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jubilee_state,  jubileep_interrupt)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jubilee_state, screen_update_jubileep)

	MCFG_GFXDECODE_ADD("gfxdecode", jubileep)

	MCFG_PALETTE_LENGTH(256)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK, mc6845_intf)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( jubileep )
	ROM_REGION( 0x4000, "maincpu", 0 ) /* TMS9980 code */
	ROM_LOAD( "1_ic59.bin", 0x0000, 0x1000, CRC(534c81c2) SHA1(4ce1d4492de9cbbc37e5a946b1183d8e8b0ba989) )
	ROM_LOAD( "2_ic58.bin", 0x1000, 0x1000, CRC(69984028) SHA1(c919a5cb43f23a0d9e496107997c74799709b347) )
	ROM_LOAD( "3_ic57.bin", 0x2000, 0x1000, CRC(c9ae423d) SHA1(8321e3e6fd60d92202b0c7b47e2a333a567b5c22) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "ic49.bin",   0x0000, 0x2000, CRC(ec65d259) SHA1(9e82e4043cbea26b91965a19507a5f00dc3ba01a) )
	ROM_LOAD( "ic48.bin",   0x2000, 0x2000, CRC(74e9ffd9) SHA1(7349fea72a349a58014b795ec6c29647e7159d39) )
	ROM_LOAD( "ic47.bin",   0x4000, 0x2000, CRC(55dc8482) SHA1(53f22bd66e5fcad5e2397998bc58109c3c19af96) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT  ROT    COMPANY    FULLNAME                  FLAGS */
GAME( 198?, jubileep, 0,      jubileep, jubileep, driver_device, 0,    ROT0, "Jubilee", "Jubilee Double-Up Poker", GAME_NO_SOUND | GAME_NOT_WORKING )
