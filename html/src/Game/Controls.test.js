/*
Automated test for Controls component
*/
import React from 'react';
import Controls from './Controls.js';

it("Idle renders correctly", function() {
  const tree =renderer.create(
	   <Controls />
	).toJSON();

	expect(tree).toMatchSnapshot();
});
