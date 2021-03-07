module AXIRAM #(
    parameter int unsigned AXI_ADDR_WIDTH = 32,
    parameter int unsigned AXI_DATA_WIDTH = 32,
    parameter int unsigned AXI_ID_WIDTH   = 10,
    parameter int unsigned AXI_STRB_WIDTH = AXI_DATA_WIDTH / 8,
    parameter int unsigned AXI_USER_WIDTH = 6
)
(
  input logic clk,
  input logic rst_ni,
  input logic [AXI_ADDR_WIDTH-1:0] aw_addr,
  input logic [2:0]                aw_prot,
  input logic [3:0]                aw_region,
  input logic [7:0]                aw_len,
  input logic [2:0]                aw_size,
  input logic [1:0]                aw_burst,
  input logic                      aw_lock,
  input logic [3:0]                aw_cache,
  input logic [3:0]                aw_qos,
  input logic [AXI_ID_WIDTH-1:0]   aw_id,
  input logic [AXI_USER_WIDTH-1:0] aw_user,
  output logic                     aw_ready,
  input logic                      aw_valid,

  input logic [AXI_ADDR_WIDTH-1:0] ar_addr,
  input logic [2:0]                ar_prot,
  input logic [3:0]                ar_region,
  input logic [7:0]                ar_len,
  input logic [2:0]                ar_size,
  input logic [1:0]                ar_burst,
  input logic                      ar_lock,
  input logic [3:0]                ar_cache,
  input logic [3:0]                ar_qos,
  input logic [AXI_ID_WIDTH-1:0]   ar_id,
  input logic [AXI_USER_WIDTH-1:0] ar_user,
  output logic                     ar_ready,
  input logic                      ar_valid,

  input logic                      w_valid,
  input logic [AXI_DATA_WIDTH-1:0] w_data,
  input logic [AXI_STRB_WIDTH-1:0] w_strb,
  input logic [AXI_USER_WIDTH-1:0] w_user,
  input logic                      w_last,
  output logic                      w_ready,

  output logic [AXI_DATA_WIDTH-1:0] r_data,
  output logic [1:0]                r_resp,
  output logic                      r_last,
  output logic [AXI_ID_WIDTH-1:0]   r_id,
  output logic [AXI_USER_WIDTH-1:0] r_user,
  input logic                       r_ready,
  output logic                      r_valid,

  output logic [1:0]                b_resp,
  output logic [AXI_ID_WIDTH-1:0]   b_id,
  output logic [AXI_USER_WIDTH-1:0] b_user,
  input logic                       b_ready,
  output logic                      b_valid
);

AXI_BUS   #(
    .AXI_ADDR_WIDTH ( AXI_ADDR_WIDTH ),
    .AXI_DATA_WIDTH ( AXI_DATA_WIDTH ),
    .AXI_ID_WIDTH   (AXI_ID_WIDTH    ),
    .AXI_USER_WIDTH (AXI_USER_WIDTH  )

) slave;
logic                        req_o;
logic                        we_o;
logic [AXI_ADDR_WIDTH-1:0]   addr_o;
logic [AXI_DATA_WIDTH/8-1:0] be_o;
logic [AXI_DATA_WIDTH-1:0]   data_o;
logic [AXI_DATA_WIDTH-1:0]   data_i;

assign slave.aw_addr =aw_addr  ;
assign slave.aw_prot =aw_prot  ;
assign slave.aw_region =aw_region  ;
assign slave.aw_len =aw_len  ;
assign slave.aw_size =aw_size  ;
assign slave.aw_burst =aw_burst  ;
assign slave.aw_lock =aw_lock  ;
assign slave.aw_cache =aw_cache  ;
assign slave.aw_qos =aw_qos  ;
assign slave.aw_id =aw_id  ;
assign slave.aw_user =aw_user  ;
assign aw_ready = slave.aw_ready ;
assign slave.aw_valid =aw_valid  ;
assign slave.ar_addr =ar_addr  ;
assign slave.ar_prot =ar_prot  ;
assign slave.ar_region = ar_region;
assign slave.ar_len =ar_len  ;
assign slave.ar_size =ar_size  ;
assign slave.ar_burst =ar_burst  ;
assign slave.ar_lock =ar_lock  ;
assign slave.ar_cache =ar_cache  ;
assign slave.ar_qos =ar_qos  ;
assign slave.ar_id =ar_id  ;
assign slave.ar_user =ar_user  ;
assign ar_ready = slave.ar_ready ;
assign slave.ar_valid =ar_valid  ;
assign slave.w_valid =w_valid  ;
assign slave.w_data =w_data ;
assign slave.w_strb =w_strb  ;
assign slave.w_user =w_user  ;
assign slave.w_last =w_last  ;
assign w_ready = slave.w_ready ;
assign r_data = slave.r_data  ;
assign r_resp = slave.r_resp  ;
assign r_last = slave.r_last  ;
assign r_id = slave.r_id ;
assign r_user = slave.r_user  ;
assign slave.r_ready = r_ready;
assign r_valid = slave.r_valid  ;
assign b_resp = slave.b_resp ;
assign b_id = slave.b_id;
assign b_user = slave.b_user;
assign slave.b_ready = b_ready;
assign b_valid = slave.b_valid ;

axi2mem #(
     .AXI_ID_WIDTH   (AXI_ID_WIDTH),
     .AXI_ADDR_WIDTH (AXI_ADDR_WIDTH),
     .AXI_DATA_WIDTH (AXI_DATA_WIDTH),
     .AXI_USER_WIDTH (AXI_USER_WIDTH)
)mem_intf(
   .clk_i (clk),    // Clock
   .rst_ni,  // Asynchronous reset active low
   .slave,
  //RAM Interface
    .req_o,
    .we_o,
    .addr_o,
    .be_o,
    .data_o,
    .data_i
);


sp_ram #(
	.ADDR_WIDTH (AXI_ADDR_WIDTH),
	.DATA_WIDTH (AXI_DATA_WIDTH),
	.NUM_WORDS  (1024) //4KB
) ram (
	.clk,
	.en_i (req_o),
	.addr_i(addr_o),
	.wdata_i(data_o),
	.rdata_o(data_i),
	.we_i(we_o),
	.be_i(be_o)
);



endmodule
