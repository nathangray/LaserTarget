import React from 'react';
import ReactDOM from 'react-dom';
import App from './App';
import Game from './Game/Game.js';
import {shallow, mount, render} from 'enzyme'

beforeEach(() => {
	// Reset mocked fetch calls
  fetch.resetMocks()
})

it('renders without crashing', () => {
  const div = document.createElement('div');
  ReactDOM.render(<App />, div);
});

it("Fetches state on #componentDidMount", async () => {

	fetch.mockResponse(JSON.stringify(Game.State.PLAY), {status: 200})

	const app = shallow(<App />)

  await app.instance().componentDidMount()

	// Starts in idle
	expect(app.state()).toHaveProperty('state', Game.State.IDLE);
});

it('Calls server to update state on change', () => {
	// Setup mock
	fetch.mockResponse(JSON.stringify(Game.State.PLAY), {status: 200});

	// Create app
	const app = shallow(<App />)

	// Try state change
	app.instance().stateChangeHandler(Game.State.PLAY);

	// Check server got right state
	expect(fetch).toBeCalledWith('/state', expect.objectContaining({body: ''+Game.State.PLAY}));
});

it('Updates state according to what server says', () => {
	// Create app
	const app = shallow(<App />)

	// Setup mocks
	fetch.mockResponse(JSON.stringify(Game.State.PLAY), {status: 200});
	spyOn(app.instance(), 'setState').and.callThrough();
	app.update();

	// Try state change - return promise to handle errors
	return app.instance().fetchState().then(() => {
			// Async check
			expect(fetch).toBeCalledWith('/state');
			expect(app.instance().setState).toBeCalled();
			expect(app.state()).toHaveProperty('state', Game.State.PLAY);
		})
});
