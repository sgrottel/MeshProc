
for (a=[0:120:359])
{
    rotate([0, 0, a])
    translate([-10, 0, 0])
    rotate([20, 0, -70])
    translate([-1, -1, -1])
    cube([2, 20, 2]);
}
