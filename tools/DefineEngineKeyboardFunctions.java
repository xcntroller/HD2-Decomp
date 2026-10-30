// Verify graphics input slots before read-only keyboard exports.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class DefineEngineKeyboardFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x10000000L)
            throw new IllegalArgumentException("Expected fingerprinted engine");
        long[][] slots = {{0x14, 0x1006a470L}, {0x9c, 0x1006f190L},
                          {0xa0, 0x1006f320L}, {0xa4, 0x1006f470L},
                          {0xa8, 0x1006f2d0L}, {0xac, 0x1006f300L},
                          {0xb0, 0x1006f490L}, {0xbc, 0x1006f550L},
                          {0xb4, 0x1006f4b0L}, {0xb8, 0x1006f500L},
                          {0xc0, 0x1006f570L},
                          {0x90, 0x1006f040L}, {0x94, 0x1006f150L},
                          {0x98, 0x1006f160L}, {0xc4, 0x1006f590L},
                          {0xc8, 0x1006f5b0L}, {0xcc, 0x1006f660L},
                          {0xd0, 0x1006f6a0L}};
        for (long[] slot : slots)
            if (Integer.toUnsignedLong(getInt(toAddr(0x1009d840L + slot[0]))) != slot[1])
                throw new IllegalStateException("Unexpected keyboard graphics slot");
        for (long[] slot : slots) {
            Address address = toAddr(slot[1]);
            if (getFunctionAt(address) == null) {
                if (getFunctionContaining(address) != null)
                    throw new IllegalStateException("Overlapping function at " + address);
                if (getInstructionAt(address) == null && !disassemble(address))
                    throw new IllegalStateException("Cannot disassemble " + address);
                if (createFunction(address, null) == null)
                    throw new IllegalStateException("Cannot define " + address);
            }
            println("Engine keyboard target: " + address);
        }
    }
}
