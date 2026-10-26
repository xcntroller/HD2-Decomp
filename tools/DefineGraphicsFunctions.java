// Define verified graphics-interface targets for transient, read-only exports.
// @category HD2

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DefineGraphicsFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x10000000L) {
            throw new IllegalArgumentException("Expected the original LS3DF.dll");
        }
        long[][] slots = {
            {0x00, 0x1006c2d0L}, {0x04, 0x1006da30L},
            {0x08, 0x1006c2c0L}, {0x0c, 0x1006da20L},
            {0x10, 0x1006da40L}, {0x14, 0x1006a470L},
            {0x18, 0x1006de50L}, {0x1c, 0x1006de70L},
            {0x48, 0x1006a240L}, {0x64, 0x1006a6c0L},
            {0x68, 0x1006b380L}, {0x6c, 0x1006b3b0L},
            {0x70, 0x1006b3e0L}, {0x9c, 0x1006f190L},
            {0xa0, 0x1006f320L}
        };
        for (long[] slot : slots) {
            Address address = toAddr(0x1009d840L + slot[0]);
            if (Integer.toUnsignedLong(getInt(address)) != slot[1]) {
                throw new IllegalStateException("Unexpected graphics vtable slot " + address);
            }
        }
        for (long[] slot : slots) {
            Address address = toAddr(slot[1]);
            if (getFunctionAt(address) == null) {
                Function containing = getFunctionContaining(address);
                if (containing != null) {
                    throw new IllegalStateException(address + " overlaps " + containing.getEntryPoint());
                }
                if (getInstructionAt(address) == null && !disassemble(address)) {
                    throw new IllegalStateException("Cannot disassemble graphics method " + address);
                }
                if (createFunction(address, null) == null) {
                    throw new IllegalStateException("Cannot define graphics method " + address);
                }
            }
            println("Graphics target: " + address);
        }
    }
}
