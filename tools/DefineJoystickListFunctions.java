// Transient definitions only; invoke with -readOnly -noanalysis.
// @category HD2
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;

public class DefineJoystickListFunctions extends GhidraScript {
    @Override
    public void run() throws Exception {
        if (!"8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307"
                .equals(currentProgram.getExecutableSHA256())
                || currentProgram.getImageBase().getOffset() != 0x400000L)
            throw new IllegalArgumentException("Expected fingerprinted client");
        long[][] anchors = {
            {0x6dc5e0, 0x56, 0x33, 0xf6, 0x8b, 0x0d, 0x3c, 0xea, 0x8a, 0x00},
            {0x6dc640, 0x83, 0xec, 0x10, 0x53, 0x57, 0xe8, 0x64, 0x88, 0x10, 0x00},
            {0x7f8f30, 0xff, 0x25, 0x60, 0xf0, 0x80, 0x00}
        };
        for (long[] anchor : anchors)
            for (int index = 1; index < anchor.length; ++index)
                if (Byte.toUnsignedInt(getByte(toAddr(anchor[0] + index - 1))) != anchor[index])
                    throw new IllegalStateException("Unexpected joystick instruction bytes");
        for (long entry : new long[]{0x6dc5e0, 0x6dc640}) {
            Address address = toAddr(entry);
            if (getFunctionAt(address) == null) {
                if (getFunctionContaining(address) != null)
                    throw new IllegalStateException("Overlapping joystick function " + address);
                if (getInstructionAt(address) == null && !disassemble(address))
                    throw new IllegalStateException("Cannot disassemble " + address);
                if (createFunction(address, null) == null)
                    throw new IllegalStateException("Cannot define " + address);
            }
            println("Joystick list target: " + address);
        }
    }
}
