
module PacketFilter #(
    parameter logic[7:0] XMax = 8'd3,
    parameter logic[7:0] YMax = 8'd3,
    parameter int TimeoutMax = 10
)
(

    input logic clk,
    input logic reset,
    
    input logic rx,
    input logic[31:0] data_in,
    output logic credit_o,
    
    output logic tx,
    output logic[31:0] data_out,
    input logic credit_i

);

// 2 FIFO registers (A & B)
logic[31:0] A;
logic AValid;
logic AReady;
logic AIsHeader;
logic AEnable;  // Controlled by FSM
logic AClear;  // Controlled by FSM

logic[31:0] B;
logic BValid;
logic BReady;
logic BIsHeader;
logic BEnable;  // Controlled by FSM
logic BClear;  // Controlled by FSM

// Timeout counter (counts how many consecutive cycles an attempt to write a flit into A was not performed)
logic timeoutEnable;
logic timeoutFlag;
logic[$clog2(TimeoutMax)-1:0] timeoutCounter;

// Flit counter (counts how many payload flits are yet to be transmitted in a packet whose header has been validate via checksums)
logic[23:0] size;

logic paddingEnable;

logic[7:0] addressChecksum;
logic[7:0] sizeChecksum;
logic validAddressChecksum;
logic validSizeChecksum;

typedef enum logic [1:0] {Saddr, Ssize, Spayload, Spad} state_t;
state_t state;

// logic invalidateBuffer;
logic txEnable;

// Combinational control to A enable
assign AReady = ((AValid && BReady) || !AValid) && AEnable ? 1'b1 : 1'b0;

// A write control
always_ff @(posedge clk) begin

	if (reset) begin
		AValid <= 1'b0;
		AIsHeader <= 1'b0;
	end else begin 

		if (rx && AReady) begin

			A <= data_in;

			if (!AClear) begin
				// AIsHeader <= (state == Saddr || state == Ssize || ((state == Spayload || state == Spad) 
				AIsHeader <= (state == Saddr || (state == Ssize && !validSizeChecksum) || ((state == Spayload || state == Spad) 
																                       && ( (size == 2 && AValid && BValid) 
																                       || (size == 1 && (AValid ^ BValid) ) ) ) ) ? 1'b1 : 1'b0;
			end

		end

		if (rx && AReady)
			AValid <= rx;
		// else if (AClear) begin
		// else if (AClear || (!rx && AReady)) begin
		else if (AClear || (!(rx && AReady) && AValid && BReady)) begin
			AValid <= 1'b0;
			AIsHeader <= 1'b0;
		end

	end

end

// Combinational control to B write enable
assign BReady = ((BValid && credit_i) || !BValid) && BEnable ? 1'b1 : 1'b0;

// B write control
always_ff @(posedge clk) begin

	if (reset) begin
		BValid <= 1'b0;
		BIsHeader <= 1'b0;
	end else begin 

		if (AValid && BReady) begin

			B <= A;

			if (!BClear) 
				BIsHeader <= AIsHeader;

		end

		if (AValid && BReady)
			BValid <= AValid;
		// else if (BClear || (!AValid && BReady)) begin
		// Consumiu e nao escreveu
		else if (BClear || (!(AValid && BReady) && BValid && (credit_i && txEnable))) begin
			BValid <= 1'b0;
			BIsHeader <= 1'b0;
		end
		
	end

end

assign tx = (BValid && (txEnable || validSizeChecksum) || paddingEnable) ? 1'b1 : 1'b0;
assign data_out = paddingEnable ? 32'd0 : B;
assign credit_o = ((rx && AReady) || !rx) ? 1'b1 : 1'b0;

// Combinationally determine valid checksums from flit written into A FIFO register
localparam logic[15:0] checksumIV = {XMax, YMax};

for (genvar i = 0; i < 8; i++)
	assign addressChecksum[i] = A[2*i] ^ A[(2*i) + 1] ^ checksumIV[2*i] ^ checksumIV[(2*i) + 1];

for (genvar i = 0; i < 8; i++)
	assign sizeChecksum[i] = A[3*i] ^ A[(3*i) + 1] ^ A[(3*i) + 2] ^ B[i+16];

assign validAddressChecksum = ((A[23:16] == addressChecksum) && AValid) ? 1'b1 : 1'b0;
assign validSizeChecksum = ((A[31:24] == sizeChecksum) && AIsHeader && AValid && BValid && (state == Ssize)) ? 1'b1 : 1'b0;

// Timeout Counter
always_ff @(posedge clk) begin

    if (reset)
        timeoutCounter <= TimeoutMax - 1;

    else begin

		timeoutFlag <= 1'b0;
    
        if (timeoutEnable) begin
		
			if (rx == 1'b1) begin
				timeoutCounter <= TimeoutMax - 1;
	
			end else begin
        
                if (!timeoutFlag)
				    timeoutCounter <= timeoutCounter - 1;
		
				if (timeoutCounter == 1) begin
					timeoutCounter <= TimeoutMax - 1;
					timeoutFlag <= 1'b1;
				end 
					
			end
			
        end
    
    end

end

// Control FSM
always_ff @(posedge clk) begin

    if (reset) begin 
	
        // invalidateBuffer <= 1'b0;
        txEnable <= 1'b0;
		
        timeoutEnable <= 1'b0;
        paddingEnable <= 1'b0;

        // size <= 16'd0;
        size <= 24'd0;

		AEnable <= 1'b1;
		AClear <= 1'b0;

		BEnable <= 1'b1;
		AClear <= 1'b0;
		
        state <= Saddr;
		
    end else begin 

		// Buffer enables active by default
		AEnable <= 1'b1;
		AClear <= 1'b0;
		BEnable <= 1'b1;
		BClear <= 1'b0;
        
		// Wait for a new valid ADDR flit
        if (state == Saddr) begin 
	
            // Remains waiting for an ADDR flit until a valid ADDR flit checksum is seen. Non-valid ADDR flits are discarted and not propagated to Router local port.
            if (validAddressChecksum) begin
                timeoutEnable <= 1'b1;
                state <= Ssize;
            end
            
		// Wait for a new SIZE flit and monitor for timeout
        end else if (state == Ssize) begin
			
			// Timed-out waiting for a SIZE flit. 
			if (timeoutFlag) begin

				timeoutEnable <= 1'b0;
				BClear <= 1'b1;
				state <= Saddr;
				
			end else begin
        
                // Valid SIZE flit, wait for first payload flit
				if (AValid && validSizeChecksum) begin

					txEnable <= 1'b1;
					size <= A[23:0];
					state <= Spayload;

                // SIZE checksum fail, invalidade ADDR flit on B and SIZE flit on A and wait for new valid ADDR flit
				end else if (AValid && !validSizeChecksum) begin

					// invalidateBuffer <= 1'b1;
					AClear <= 1'b1;
					BClear <= 1'b1;
					state <= Saddr;

					$display("Dropping packet due to SIZE checksum error");

				end
					
			end
		
		// Transmit packet payload and monitor for timeout
		end else if (state == Spayload) begin
		
			// Timed-out waiting for a flit
			if (timeoutFlag) begin
			
				timeoutEnable <= 1'b0;
				paddingEnable <= 1'b1;
				BEnable <= 1'b0;

				state <= Spad;
				
            // Payload flit in B register was consumed by Router local port
			// end else if (BIsValid && !BIsHeader && credit_i) begin
			end else if (BValid && !BIsHeader && credit_i) begin
			
				size <= size - 1;
				
                // Last payload flit sent, wait for ADDR flit to be written to A
				if (size == 1) begin

                    txEnable <= 1'b0;
					timeoutEnable <= 1'b0;
				
					// Skip to waiting for SIZE flit if there already is a valid ADDR flit in A
					// if (AIsValid && validAddressChecksum)
					if (AValid && validAddressChecksum)
						state <= Ssize;

				    else begin

                        if (!validAddressChecksum)
                            // invalidateBuffer <= 1'b1;
                            AClear <= 1'b1;

					    state <= Saddr;

		            end
			
				end	
			
			end
	
		// Pad remainder of a packet's payload with null flits after timeout
		end else if (state == Spad) begin

			BEnable <= 1'b0;
		    
			// TX is always '1' when inserting padding
            if (credit_i) begin

			    size <= size - 1;
			    
                // Last padding flit sent, wait for ADDR flit to be written to B
			    if (size == 1) begin
			    
                    txEnable <= 1'b0;
				    paddingEnable <= 1'b0;
					BEnable <= 1'b1;
					
					// Skip to waiting for SIZE flit if there already is a valid ADDR flit in A
				    // if (AIsValid && validAddressChecksum)
				    if (AValid && validAddressChecksum)
					    state <= Ssize;

				    else begin

                        if (!validAddressChecksum)
                            // invalidateBuffer <= 1'b1;
							AClear <= 1'b1;

					    state <= Saddr;

		            end

			    end	

		    end

        end 
    
    end
	
end

// TODO: `ifdef DEBUG

always @(posedge clk) begin

	if (AValid && AIsHeader && !validAddressChecksum && state == Saddr)
		$display("Dropping packet due to ADDR checksum error");

end

endmodule
