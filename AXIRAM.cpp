#include <stdio.h>
#include <stdlib.h>
#include "VAXIRAM.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

int	tickcount = 0;
int     data_24 = 0;
VAXIRAM *tb;
VerilatedVcdC * tfp;

typedef struct {
	unsigned int data;
	unsigned int strb;
} WRITE_PACKET;

void tick() {
	tickcount++;
	//tb->eval();
	//if(tfp)
		//tfp->dump(tickcount * 10 - 2);
	tb->clk = 1;
	tb->eval();
	if(tfp)
		tfp->dump(tickcount * 10);
	tb->clk = 0;
	tb->eval();
	if(tfp) {
		tfp->dump(tickcount * 10 + 5);
		tfp->flush();
	}
}

void set_aw(int addr, int len, int type) {
	tb->aw_addr = addr;
	tb->aw_prot = 0x0;
	tb->aw_region = 0x0;
	tb->aw_len = len; //Single transaction
	tb->aw_size = 0x2; // 32 bit transaction
	tb->aw_burst = type; //Single
	tb->aw_lock = 0x0;
	tb->aw_cache = 0x0;
	tb->aw_qos=0x0;
	tb->aw_id = 0x0;
	tb->aw_user = 0x0;
	tb->aw_valid = 0x1; // send AW Channel Transaction
}

void set_w(int strb, int data, int last) {
	//make Write Access
	tb->w_valid = 0x1;
	tb->w_strb = strb;
	tb->w_last = last;
	tb->w_user = 0x0; //Single transaction
	tb->w_data = data; // Write data
}

void clear_aw() {
	tb->aw_addr =0;
	tb->aw_prot =0;
	tb->aw_region =0;
	tb->aw_len =0; //Single transaction
	tb->aw_size =0; // 32 bit transaction
	tb->aw_burst =0; //Single
	tb->aw_lock =0;
	tb->aw_cache =0;
	tb->aw_qos=0;
	tb->aw_id =0;
	tb->aw_user =0;
	tb->aw_valid =0; // send AW Channel Transaction
}

void clear_w() {
	tb->w_valid = 0x0;
	tb->w_strb = 0x0;
	tb->w_last = 0x0;
	tb->w_user = 0x0; //Single transaction
	tb->w_data = 0x0; // Write 2 
}


//type
//0 - FIXED
//1 - INCR
//2 - WRAP
void makeWrite(int addr, int* data ,int* strb, int count, int type) {
	//Make AR Request
	
	//In AXI, transaction count - 1 becomes AxLEN value
	set_aw(addr,count - 1, type);

	if(count == 1)
		set_w(strb[count-1], data[count-1], 1);//this is last transaction
	else 
		set_w(strb[count-1], data[count-1], 0);//this is last transaction
	//we send AW & W in the same time, so we need to check ready signal both
	while(tb->aw_ready != 0x1 && tb->w_ready != 0x1) {
		tick();
		//tfp->dump(k);
	}

	//while(tb->aw_ready != 0x1) {
		//tick();
		////tfp->dump(k);
	//}
	//Valid Signal comes out from the axi2mem controller
	
	printf("[SIM_INFO] You checked aw_ready up!\n");
	printf("[SIM_INFO] You checked %dnd w_ready up!\n", count-1);

	clear_aw();
	clear_w();
	tick();

	count--; //we decrease count value because we already write 
	for(;count !=0;) {
		if(count == 0) break; //for avoid sending single transaction again
		if(count == 1)
			set_w(strb[count-1], data[count-1], 1);//this is last transaction
		else 
			set_w(strb[count-1], data[count-1], 0);//this is last transaction
		tick();
		printf("count value %d\n", count);
		while(tb->w_ready != 0x1) {
			tick();
			//tfp->dump(k);
		}
		//Valid Signal comes out from the axi2mem controller

		printf("[SIM_INFO] You checked %dnd w_ready up!\n", count-1);
		clear_w();
		tick();
		count--;
	}

	//check B signal
	while(tb->b_valid != 0x1) {
		tick( );
		//tfp->dump(k);
	}

	printf("You checked b_valid up!\n");

	tb->b_ready=0x1;

	tick();

	printf("AXI Write Transaction Done!\n");

	if(count != 0) {
		printf("[SIM_ERROR] AXI Write Transaction not Finish all\n");
	}
}

void set_ar(int addr, int count, int type) {

	tb->ar_addr = addr;
	tb->ar_prot = 0x0;
	tb->ar_region = 0x0;
	tb->ar_len = count; 
	tb->ar_size = 0x2; // 32 bit transaction
	tb->ar_burst = type; //Single
	tb->ar_lock = 0x0;
	tb->ar_cache = 0x0;
	tb->ar_qos=0x0;
	tb->ar_id = 0x0;
	tb->ar_user = 0x0;
	tb->ar_valid = 0x1; // send AW Channel Transaction
}

void clear_ar()
{
	tb->ar_addr =0;
	tb->ar_prot =0;
	tb->ar_region =0;
	tb->ar_len =0; //Single transaction
	tb->ar_size =0; // 32 bit transaction
	tb->ar_burst =0; //Single
	tb->ar_lock =0;
	tb->ar_cache =0;
	tb->ar_qos=0;
	tb->ar_id =0;
	tb->ar_user =0;
	tb->ar_valid =0; // send AW Channel Transaction

}

int  makeRead(int addr, int count, int type) {
	//Make AR Request
	int read_data;

	set_ar(addr, count - 1, type);
	


	while(tb->ar_ready != 0x1) {
		tick();
		//tfp->dump(k);
	}
	//Valid Signal comes out from the axi2mem controller

	clear_ar();

	printf("You checked ar_ready up!\n");


	while (count != 0) {
		while(tb->r_valid != 0x1) {
			tick( );
			//tfp->dump(k);
		}

		printf("You checked r_valid up!\n");

		tb->r_ready=0x1;

		printf("[SIM_INFO] the read data is %08x\n",tb->r_data);
		read_data = tb->r_data;

		tick();
		count--;
	}

	printf("AXI Read Transaction Done!\n");
	//for Single Read, you return read_data
	//for others, you return none
	return read_data;
}

int main(int argc, char ** argv) {
	int result;
	int data[3], strb[3];
	Verilated::commandArgs(argc, argv);
	Verilated::traceEverOn(true);
	tb = new VAXIRAM();
	tfp = new VerilatedVcdC;
	tb->trace(tfp, 99);
	tfp->open("obj_dir/sim.vcd");

	//initialize test data
	
	data[0] = 0x5A5A0088, strb[0] = 0x3;
	data[1] = 0xA5A5FF77, strb[1] = 0xC;
	data[2] = 0xFFFFFFFF, strb[2] = 0xA;
	
	//Reset Sequence starts from the beginning of the testbench
	tb->rst_ni = 0; 
	for (int i = 0; i < 50; i++)
		tick( );
	tb->rst_ni = 1; //release Reset

	for (int i = 0; i < 10; i++)
		tick( );


	makeWrite(0x00000010,data, strb, 2, 0); //FIXED(single transaction)
	makeWrite(0x00000020,data, strb, 3, 1); //Increment

	result = makeRead(0x00000010, 1, 0);
	
	if(result != 0xA5A50088) {
		printf("[SIM_ERROR] Addr 0x10 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x10 Write test Success\n");


	result = makeRead(0x00000020, 1, 0);
	if(result != 0xFF00FF00) {
		printf("[SIM_ERROR] Addr 0x20 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x20 Write test Success\n");

	result = makeRead(0x00000024, 1, 0);
	if(result != 0xA5A50000) {
		printf("[SIM_ERROR] Addr 0x24 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x24 Write test Success\n");

	result = makeRead(0x00000028, 1, 0);
	if(result != 0x00000088) {
		printf("[SIM_ERROR] Addr 0x28 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x28 Write test Success\n");

	
	data[0] = 0x00112233, strb[0] = 0xF;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x00112233) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[0] = 0x44556677, strb[0] = 0xC;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x44552233) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[0] = 0x44556677, strb[0] = 0x3;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x44556677) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[0] = 0x8899AABB, strb[0] = 0x8;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x88556677) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[0] = 0x8899AABB, strb[0] = 0x4;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x88996677) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[0] = 0x8899AABB, strb[0] = 0x2;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x8899AA77) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[0] = 0x8899AABB, strb[0] = 0x1;

	makeWrite(0x00000040, data, strb, 1, 0);

	result = makeRead(0x00000040, 1, 0);
	if(result != 0x8899AABB) {
		printf("[SIM_ERROR] Addr 0x40 Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x40 Write test Success\n");

	data[3] = 0x1F2F3F4F, strb[3] = 0x8;
	data[2] = 0x5F6F7F8F, strb[2] = 0x4;
	data[1] = 0xA5A5FF77, strb[1] = 0x2;
	data[0] = 0x5A5A0088, strb[0] = 0x1;

	makeWrite(0x00000128,data, strb, 4, 2); //WRAP transaction

	result = makeRead(0x00000120, 1, 0);
	if(result != 0x0000FF00) {
		printf("[SIM_ERROR] Addr 0x120 Wrap Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x120 Wrap Write test Success\n");

	result = makeRead(0x00000124, 1, 0);
	if(result != 0x00000088) {
		printf("[SIM_ERROR] Addr 0x124 Wrap Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x124 Wrap Write test Success\n");

	result = makeRead(0x00000128, 1, 0);
	if(result != 0x1F000000) {
		printf("[SIM_ERROR] Addr 0x128 Wrap Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x128 Wrap Write test Success\n");

	result = makeRead(0x0000012C , 1, 0);
	if(result != 0x006F0000) {
		printf("[SIM_ERROR] Addr 0x12C Wrap Write test failed!\n"); 
		printf("[SIM_ERROR] Read Data : %08x\n", result); 
	} else 
		printf("[SIM_INFO] Addr 0x12C Wrap Write test Success\n");

	result = makeRead(0x00000128, 4, 2); //WRAP Read

	result = makeRead(0x00000128, 8, 2); //WRAP Read

	result = makeRead(0x00000128, 16, 2); //WRAP Read
	tfp->close();
	delete tb;
	delete tfp;
}
