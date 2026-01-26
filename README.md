README for Brad and Lukke's Gradient Mesh Project

We implemented the main task of the assignement - the creation of the basic graident mesh. We also implemented the bonus of the sectored control points for sharp colour transitions.

You can open and run the project in QTcreator - the current cmake file is for Brad's windows setup so may need to be modified a bit if you're running on Linux.

We expanded what we showed in the demo by also visualising the tangent handles as part of the control net. You can click and drag the tangents to change them and this influences the curves and therefore 
the colour spread of the gradient mesh. Be aware you can only drag the top and right tangent for each control point. Moving this will also modify the opposite tangent on that same control point. This gives C1
continuity across the patches. You have to grab the tip of the tangent line too, can be a bit fiddly

We have some preset meshes to choose from, with the last one showing the sectored control points. You can toggle between viewing the gradient mesh or just the control points. You can also toggle 
between having straight lines betwen the control points or showing the hermite curves.

we also fixed the bug from the demo where moving control points can influence the curves on other patches.
