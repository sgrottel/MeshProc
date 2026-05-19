
difference()
{
    sphere(r = 10, $fn = 32);
    sphere(r = 9, $fn = 32);
    cylinder(h = 11, r = 2, $fn = 12);
}

translate([25, 0, 0])
difference()
{
    sphere(r = 10, $fn = 32);
    sphere(r = 9, $fn = 32);
    translate([0, 0, -11])
    cylinder(h = 22, r = 2, $fn = 12);
}

translate([-25, 0, 0])
difference()
{
    sphere(r = 10, $fn = 32);
    sphere(r = 9, $fn = 32);
    translate([0, 0, -11])
    cylinder(h = 22, r = 2, $fn = 12);
    rotate([90, 0, 0])
    cylinder(h = 11, r = 2, $fn = 12);
}
