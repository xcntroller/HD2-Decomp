// Verify video vtables before defining indirect methods in a read-only session.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;

public class DefineOpeningVideoFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        String sha = currentProgram.getExecutableSHA256();
        long table;
        long[] targets;
        if ("12c61eed2aec0c45700ad0a5ddfd7cbb3cbf0ea15ec67de155790bb24e7756ee".equals(sha)
                && currentProgram.getImageBase().getOffset() == 0x10000000L) {
            table = 0x1009d9c0L;
            targets = new long[]{0x10070fc0L, 0x10070fd0L, 0x10071b70L,
                0x10071630L, 0x10071b80L, 0x10071c50L, 0x10071d10L,
                0x10071da0L, 0x10072040L, 0x100720a0L, 0x100720e0L,
                0x10072160L, 0x100722a0L, 0x10072260L, 0x10072400L,
                0x100724d0L, 0x100732f0L, 0x100725a0L, 0x100725b0L,
                0x100725d0L, 0x10072650L};
        } else if ("8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307".equals(sha)
                && currentProgram.getImageBase().getOffset() == 0x400000L) {
            table = 0x815bc8L;
            targets = new long[]{0x6c0dc0L, 0x688040L, 0x66ed30L,
                0x66ed50L, 0x67ec60L, 0x66ed80L, 0x66eda0L, 0x6c0db0L};
        } else {
            throw new IllegalArgumentException("Expected fingerprinted base client or engine");
        }
        for (int i = 0; i < targets.length; ++i)
            if (Integer.toUnsignedLong(getInt(toAddr(table + i * 4))) != targets[i])
                throw new IllegalStateException("Unexpected opening video vtable");
        for (long target : targets) {
            Address address = toAddr(target);
            Function function = getFunctionAt(address);
            if (function == null) {
                if (getFunctionContaining(address) != null)
                    throw new IllegalStateException("Overlapping function at " + address);
                if (getInstructionAt(address) == null && !disassemble(address))
                    throw new IllegalStateException("Cannot disassemble " + address);
                if (createFunction(address, null) == null)
                    throw new IllegalStateException("Cannot define " + address);
            }
            println("Opening video target: " + address);
        }
    }
}
