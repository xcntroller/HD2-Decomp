// Verify original GameData reader slots before transient read-only exports.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class DefineGameDataFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x400000L)
            throw new IllegalArgumentException("Expected original base client");
        long[][] slots = {{0x815a50L, 0x6bd4f0L}, {0x815a4cL, 0x6bab00L},
                          {0x815a44L, 0x6b4f70L}};
        for (long[] slot : slots) {
            if (Integer.toUnsignedLong(getInt(toAddr(slot[0]))) != slot[1])
                throw new IllegalStateException("Unexpected GameData slot");
            Address address = toAddr(slot[1]);
            if (getFunctionAt(address) == null) {
                if (getFunctionContaining(address) != null)
                    throw new IllegalStateException("Overlapping function at " + address);
                if (getInstructionAt(address) == null && !disassemble(address))
                    throw new IllegalStateException("Cannot disassemble " + address);
                if (createFunction(address, null) == null)
                    throw new IllegalStateException("Cannot define " + address);
            }
            println("Verified GameData target: " + address);
        }
    }
}
